#include <iostream>
#include <string>
#include "logger.h"
#include "db_factory.h"
#include "query_result.h"
#include <set>
db *db_factory::singleton_instance=NULL;

int main(int argc, char* argv[]){

	db *sqlite=NULL;
	query_result *shifted_gcat_entries=NULL;

	if(argc<2){
		std::cerr<<"Usage: adjust_upenn_morphdb /path/to/morph/db/file"<<std::endl;
		exit(EXIT_FAILURE);
	}
	std::string db_file=argv[1];
	sqlite=db_factory::get_instance();
	sqlite->open(db_file);
	//As a corollary of the conversion, the morphology db table schema and the structure of
	//the original text file, feature_nr=0 shall contain the stem, feature_nr=1 shall contain the gcat
	//and feature_nr=2 and above contain other features.
	//In the original text file gcat values appear right after the stem i.e in the db at feature_nr=1.
	//However, in certain cases it is shifted as the authors wanted to break
	//up e.g. "we're" into "we" and "'re" so they put this second part in the next field
	//after the stem (feature_nr=1) and only after that comes the gcat (feature_nr=2).
	//So move all gcats to their place i.e. to feature_nr=1.
	//The following hardcoded list of gcat values were extracted from the
	//morphology db after the conversion from the original text file
	//by issuing "select distinct feature from morphology where feature_nr=1;"
	std::set<std::string> gcats={"V","N","A","Adv","PropN","Prep","Part","Pron","Det","I","Conj","Punct","Comp","G"};
	std::string gcat_list="(";
	for(auto&& i:gcats){
		gcat_list+="'"+i+"',";
	}
	gcat_list.pop_back();//remove last ','
	gcat_list+=")";
	shifted_gcat_entries=sqlite->exec_sql("SELECT * FROM MORPHOLOGY WHERE FEATURE_NR=1 AND FEATURE NOT IN "+gcat_list+";");
	if(shifted_gcat_entries!=NULL){
		for(unsigned int i=0;i<shifted_gcat_entries->nr_of_result_rows();++i){
			std::string wordform=*shifted_gcat_entries->field_value_at_row_position(i,"wordform");
			std::string wordform_escaped;
			for(auto i:wordform){
				if(i=='\''){
					wordform_escaped+="''";
				}
				else{
					wordform_escaped+=i;
				}
			}
			std::string group_id=*shifted_gcat_entries->field_value_at_row_position(i,"group_id");
			std::string feature_nr;
			query_result *current_shifted_gcat_entries=sqlite->exec_sql("SELECT * FROM MORPHOLOGY WHERE WORDFORM='"+wordform_escaped+"' AND GROUP_ID='"+group_id+"' ORDER BY FEATURE_NR;");
			for(unsigned int j=0;j<current_shifted_gcat_entries->nr_of_result_rows();++j){
				if(std::stoi(*current_shifted_gcat_entries->field_value_at_row_position(j,"feature_nr"))>1){
					feature_nr=*current_shifted_gcat_entries->field_value_at_row_position(j,"feature_nr");
					std::string feature=*current_shifted_gcat_entries->field_value_at_row_position(j,"feature");
					std::string previous_feature_nr=*current_shifted_gcat_entries->field_value_at_row_position(j-1,"feature_nr");
					sqlite->exec_sql("UPDATE MORPHOLOGY SET FEATURE = '"+feature+"' WHERE WORDFORM = '"+wordform_escaped+"' AND GROUP_ID = '"+group_id+"' AND FEATURE_NR = '"+previous_feature_nr+"';");
				}
			}
			if(feature_nr.empty()==false){
				sqlite->exec_sql("DELETE FROM MORPHOLOGY WHERE WORDFORM = '"+wordform_escaped+"' AND GROUP_ID = '"+group_id+"' AND FEATURE_NR = '"+feature_nr+"';");
			}
			delete current_shifted_gcat_entries;
		}
		delete shifted_gcat_entries;
	}
	sqlite->close();
	db_factory::delete_instance();
	sqlite=NULL;
	return 0;
}
