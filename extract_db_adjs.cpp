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
	query_result *adj_entries=NULL,*max_feature_nr_entry=NULL;
	std::set<std::string> stems,irreg_comps_dcons,irreg_supers_dcons;
	std::multimap<std::string,std::pair<std::string,std::string>> irreg_comps,irreg_supers;

	if(argc<3){
		std::cerr<<"Usage: extract_db_adjs /path/to/morph/db/file lexicon_name"<<std::endl;
		exit(EXIT_FAILURE);
	}
	//logger::singleton("console",3,"LE");
	std::string db_file=argv[1];
	std::string lexicon=argv[2];
	sqlite=db_factory::get_instance();
	sqlite->open(db_file);
	adj_entries=sqlite->exec_sql("SELECT WORDFORM, GROUP_ID FROM MORPHOLOGY WHERE FEATURE_NR=1 AND FEATURE='A';");
	if(adj_entries!=NULL){
		for(unsigned int i=0;i<adj_entries->nr_of_result_rows();++i){
			std::string wordform=*adj_entries->field_value_at_row_position(i,"wordform");
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
				std::string group_id=*adj_entries->field_value_at_row_position(i,"group_id");
				query_result *current_adj_entries=sqlite->exec_sql("SELECT * FROM MORPHOLOGY WHERE WORDFORM='"+wordform_escaped+"' AND GROUP_ID='"+group_id+"' ORDER BY FEATURE_NR;");
				if(current_adj_entries!=NULL&&current_adj_entries->nr_of_result_rows()==3){
					auto feature_comp=current_adj_entries->first_value_for_field_name_found("feature","COMP");
					auto feature_super=current_adj_entries->first_value_for_field_name_found("feature","SUPER");
					if(feature_comp!=NULL){
						std::string stem=*current_adj_entries->field_value_at_row_position(0,"feature");
						if(stem.front()!='\''&&stem.front()!='-'&&stem.find_first_not_of("'-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos){
							if(std::string("aeiouy").find(stem.back())==std::string::npos&&wordform==stem+std::string(1,stem.back())+"er"){//consonant doubling
								irreg_comps_dcons.insert(stem);
							}
							else if(stem.back()=='y'&&wordform==std::string(stem).erase(stem.size()-1,1)+"ier"){
								stems.insert(stem);
							}
							else if(stem.back()=='e'&&wordform==stem+"r"){
								stems.insert(stem);
							}
							else if(stem.back()!='e'&&wordform==stem+"er"){
								stems.insert(stem);
							}
							else{
								std::string feature;
								std::string features;
								for(unsigned int k=2;k<current_adj_entries->nr_of_result_rows();++k){
									feature=*current_adj_entries->field_value_at_row_position(k,"feature");
									features+="+"+feature;
								}
								irreg_comps.insert(std::make_pair(stem,std::make_pair(wordform,features)));
							}
						}
					}
					if(feature_super!=NULL){
						std::string stem=*current_adj_entries->field_value_at_row_position(0,"feature");
						if(stem.front()!='\''&&stem.front()!='-'&&stem.find_first_not_of("'-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos){
							if(std::string("aeiouy").find(stem.back())==std::string::npos&&wordform==stem+std::string(1,stem.back())+"est"){//consonant doubling
								irreg_supers_dcons.insert(stem);
							}
							else if(stem.back()=='y'&&wordform==std::string(stem).erase(stem.size()-1,1)+"iest"){
								stems.insert(stem);
							}
							else if(stem.back()=='e'&&wordform==stem+"st"){
								stems.insert(stem);
							}
							else if(stem.back()!='e'&&wordform==stem+"est"){
								stems.insert(stem);
							}
							else{
								std::string feature;
								std::string features;
								for(unsigned int k=2;k<current_adj_entries->nr_of_result_rows();++k){
									feature=*current_adj_entries->field_value_at_row_position(k,"feature");
									features+="+"+feature;
								}
								irreg_supers.insert(std::make_pair(stem,std::make_pair(wordform,features)));
							}
						}
					}
				}
				else if(current_adj_entries!=NULL&&current_adj_entries->nr_of_result_rows()==2){
					std::string stem=*current_adj_entries->field_value_at_row_position(0,"feature");
					if(stem.front()!='\''&&stem.front()!='-'&&stem.find_first_not_of("'-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos){
						stems.insert(stem);
					}
				}
				delete current_adj_entries;
			}
		}
		delete adj_entries;
	}
	for(auto i:stems){
		if(irreg_comps_dcons.find(i)==irreg_comps_dcons.end()
			&&irreg_supers_dcons.find(i)==irreg_supers_dcons.end()
			&&irreg_comps.find(i)==irreg_comps.end()
			&&irreg_supers.find(i)==irreg_supers.end()){
			std::cout<<i<<" "<<lexicon<<";"<<std::endl;
		}
	}
	for(auto i:irreg_comps){
		std::cout<<i.first<<"[stem]+A"<<i.second.second<<":"<<i.second.first<<" #;"<<std::endl;
	}
	for(auto i:irreg_supers){
		std::cout<<i.first<<"[stem]+A"<<i.second.second<<":"<<i.second.first<<" #;"<<std::endl;
	}
	std::cout<<"\nConsonant doubling lexc:\n";
	for(auto i:stems){
		if(irreg_comps_dcons.find(i)!=irreg_comps_dcons.end()
			&&irreg_supers_dcons.find(i)!=irreg_supers_dcons.end()){
			std::cout<<i<<" "<<lexicon<<";"<<std::endl;
		}
	}
	sqlite->close();
	db_factory::delete_instance();
	sqlite=NULL;
	return 0;
}
