#include <iostream>
#include <string>
#include "logger.h"
#include "db_factory.h"
#include "query_result.h"
#include <set>
#include <map>
db *db_factory::singleton_instance=NULL;

int main(int argc, char* argv[]){

	db *sqlite=NULL;
	query_result *pron_entries=NULL,*max_feature_nr_entry=NULL;
	std::multimap<std::string,std::pair<std::string,std::string>> stems;

	if(argc<3){
		std::cerr<<"Usage: extract_db_prons /path/to/morph/db/file lexicon_name"<<std::endl;
		exit(EXIT_FAILURE);
	}
	//logger::singleton("console",3,"LE");
	std::string db_file=argv[1];
	std::string lexicon=argv[2];
	sqlite=db_factory::get_instance();
	sqlite->open(db_file);
	pron_entries=sqlite->exec_sql("SELECT WORDFORM, GROUP_ID FROM MORPHOLOGY WHERE FEATURE_NR=1 AND FEATURE='Pron';");
	if(pron_entries!=NULL){
		for(unsigned int i=0;i<pron_entries->nr_of_result_rows();++i){
			std::string wordform=*pron_entries->field_value_at_row_position(i,"wordform");
			if(wordform.front()!='\''&&wordform.front()!='-'&&wordform.find_first_not_of("'-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos){
				std::string wordform_escaped;
				for(auto j:wordform){
					if(j=='\''){
						wordform_escaped+="''";
					}
					else{
						wordform_escaped+=j;
					}
				}
				std::string group_id=*pron_entries->field_value_at_row_position(i,"group_id");
				query_result *current_pron_entries=sqlite->exec_sql("SELECT * FROM MORPHOLOGY WHERE WORDFORM='"+wordform_escaped+"' AND GROUP_ID='"+group_id+"' ORDER BY FEATURE_NR;");
				if(current_pron_entries!=NULL){
					std::string stem=*current_pron_entries->field_value_at_row_position(0,"feature");
					if(stem.front()!='\''&&stem.front()!='-'&&stem.find_first_not_of("'-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos){
						std::string feature;
						std::string features;
						for(unsigned int k=2;k<current_pron_entries->nr_of_result_rows();++k){
							feature=*current_pron_entries->field_value_at_row_position(k,"feature");
							features+="+"+feature;
						}
						stems.insert(std::make_pair(stem,std::make_pair(wordform,features)));
					}
				}
				delete current_pron_entries;
			}
		}
		delete pron_entries;
	}
	for(auto i:stems){
		std::cout<<i.first<<"[stem]+Pron"<<i.second.second<<":"<<i.second.first<<" #;"<<std::endl;
	}
	sqlite->close();
	db_factory::delete_instance();
	sqlite=NULL;
	return 0;
}
