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
	query_result *noun_entries=NULL,*max_feature_nr_entry=NULL;
	std::set<std::string> stems;
	std::multimap<std::string,std::pair<std::string,std::string>> plurals,irregular_plurals;

	if(argc<3){
		std::cerr<<"Usage: extract_db_nouns /path/to/morph/db/file lexicon_name"<<std::endl;
		exit(EXIT_FAILURE);
	}
	//logger::singleton("console",3,"LE");
	std::string db_file=argv[1];
	std::string lexicon=argv[2];
	sqlite=db_factory::get_instance();
	sqlite->open(db_file);
	noun_entries=sqlite->exec_sql("SELECT WORDFORM, GROUP_ID FROM MORPHOLOGY WHERE FEATURE_NR=1 AND FEATURE='N';");
	if(noun_entries!=NULL){
		for(unsigned int i=0;i<noun_entries->nr_of_result_rows();++i){
			std::string wordform=*noun_entries->field_value_at_row_position(i,"wordform");
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
				std::string group_id=*noun_entries->field_value_at_row_position(i,"group_id");
				query_result *current_noun_entries=sqlite->exec_sql("SELECT * FROM MORPHOLOGY WHERE WORDFORM='"+wordform_escaped+"' AND GROUP_ID='"+group_id+"' ORDER BY FEATURE_NR;");
				if(current_noun_entries!=NULL&&current_noun_entries->nr_of_result_rows()==3){
					auto feature_3sg=current_noun_entries->first_value_for_field_name_found("feature","3sg");
					auto feature_3pl=current_noun_entries->first_value_for_field_name_found("feature","3pl");
					if(feature_3sg!=NULL){
						std::string stem=*current_noun_entries->field_value_at_row_position(0,"feature");
						if(stem.front()!='\''&&stem.front()!='-'&&stem.find_first_not_of("'-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos){
							stems.insert(stem);
						}
					}
					if(feature_3pl!=NULL){
						std::string stem=*current_noun_entries->field_value_at_row_position(0,"feature");
						if(stem.front()!='\''&&stem.front()!='-'&&stem.find_first_not_of("'-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos){
							std::string feature;
							std::string features;
							for(unsigned int k=2;k<current_noun_entries->nr_of_result_rows();++k){
								feature=*current_noun_entries->field_value_at_row_position(k,"feature");
								features+="+"+feature;
							}
							plurals.insert(std::make_pair(stem,std::make_pair(wordform,features)));
						}
					}
				}
				delete current_noun_entries;
			}
		}
		delete noun_entries;
	}
	for(auto i:stems){
		auto plural=plurals.find(i);
		if(plural!=plurals.end()){
			if(plural->second.first.back()!='s'){//irregular plurals where in case of feature=3pl the wordform does NOT end in "s"
				//std::cout<<plural->first<<"[stem]+N"<<plural->second.second<<":"<<plural->second.first<<" #;"<<std::endl;
				irregular_plurals.insert(*plural);
				plurals.erase(plural);
			}
			else{//regular nouns
				std::cout<<i<<" "<<lexicon<<";"<<std::endl;
			}
		}
		else{//nouns with no plural form e.g. water
			std::cout<<i<<" "<<lexicon<<"_NoPl;"<<std::endl;
		}
	}
	for(auto i:plurals){//check plurals having no singular form
		if(stems.find(i.first)==stems.end()){
			std::cout<<i.first<<"[stem]+N"<<i.second.second<<":"<<i.second.first<<" #;"<<std::endl;
		}
	}
	for(auto i:irregular_plurals){//irregular plurals
		std::cout<<i.first<<"[stem]+N"<<i.second.second<<":"<<i.second.first<<" #;"<<std::endl;
	}
	sqlite->close();
	db_factory::delete_instance();
	sqlite=NULL;
	return 0;
}
