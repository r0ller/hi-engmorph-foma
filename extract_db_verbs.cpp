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
	query_result *verb_entries=NULL,*max_feature_nr_entry=NULL;
	std::set<std::string> stems,weak_stems,prog_stems_dcon,past_stems_dcon;
	std::multimap<std::string,std::pair<std::string,std::string>> strong_stems_past,strong_stems_ppart,present_stems;

	if(argc<3){
		std::cerr<<"Usage: extract_db_verbs /path/to/morph/db/file lexicon_name"<<std::endl;
		exit(EXIT_FAILURE);
	}
//	logger::singleton("console",3,"LE");
	std::string db_file=argv[1];
	std::string lexicon=argv[2];
	sqlite=db_factory::get_instance();
	sqlite->open(db_file);
	verb_entries=sqlite->exec_sql("SELECT STEMS.WORDFORM AS wordform, STEMS.GROUP_ID AS group_id, STEMS.FEATURE AS feature FROM (SELECT WORDFORM, GROUP_ID FROM MORPHOLOGY WHERE FEATURE_NR=1 AND FEATURE='V') AS VERBS INNER JOIN (SELECT WORDFORM, GROUP_ID, FEATURE FROM MORPHOLOGY WHERE FEATURE_NR=0) AS STEMS ON VERBS.WORDFORM=STEMS.WORDFORM AND VERBS.GROUP_ID=STEMS.GROUP_ID;");
	if(verb_entries!=NULL){
		for(unsigned int i=0;i<verb_entries->nr_of_result_rows();++i){
			std::string wordform=*verb_entries->field_value_at_row_position(i,"wordform");
			std::string stem=*verb_entries->field_value_at_row_position(i,"feature");
			if(stem.front()!='\''&&stem.front()!='-'&&stem.find_first_not_of("'-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos
					&&wordform.front()!='\''&&wordform.front()!='-'&&wordform.find_first_not_of("'-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")==std::string::npos){
				std::string wordform_escaped;
				for(auto j:wordform){
					if(j=='\''){
						wordform_escaped+="''";
					}
					else{
						wordform_escaped+=j;
					}
				}
				std::string group_id=*verb_entries->field_value_at_row_position(i,"group_id");
				query_result *stem_entries=sqlite->exec_sql("SELECT * FROM MORPHOLOGY WHERE WORDFORM='"+wordform_escaped+"' AND GROUP_ID='"+group_id+"' ORDER BY FEATURE_NR;");
				auto verb_stem_entry=stem_entries->first_value_for_field_name_found("feature","V");
				while(verb_stem_entry!=NULL){
					auto feature_strong=stem_entries->first_value_for_field_name_found("feature","STR");
					auto feature_weak=stem_entries->first_value_for_field_name_found("feature","WK");
					auto feature_inf=stem_entries->first_value_for_field_name_found("feature","INF");
					auto feature_pres=stem_entries->first_value_for_field_name_found("feature","PRES");
					auto feature_prog=stem_entries->first_value_for_field_name_found("feature","PROG");
					if(feature_strong!=NULL){
						auto feature_past=stem_entries->first_value_for_field_name_found("feature","PAST");
						auto feature_ppart=stem_entries->first_value_for_field_name_found("feature","PPART");
						if(feature_past!=NULL&&feature_ppart==NULL){
							auto upper_bound=strong_stems_past.upper_bound(stem);
							bool wordform_found=false;
							for(auto k=strong_stems_past.lower_bound(stem);k!=upper_bound;++k){
								if(k->second.first==wordform){
									wordform_found=true;
									break;
								}
							}
							if(wordform_found==false){
								std::string feature;
								std::string features;
								for(unsigned int k=2;k<stem_entries->nr_of_result_rows();++k){
									feature=*stem_entries->field_value_at_row_position(k,"feature");
									features+="+"+feature;
								}
								strong_stems_past.insert(std::make_pair(stem,std::make_pair(wordform,features)));
							}
						}
						if(feature_ppart!=NULL&&feature_past==NULL){
							auto upper_bound=strong_stems_ppart.upper_bound(stem);
							bool wordform_found=false;
							for(auto k=strong_stems_ppart.lower_bound(stem);k!=upper_bound;++k){
								if(k->second.first==wordform){
									wordform_found=true;
									break;
								}
							}
							if(wordform_found==false){
								std::string feature;
								std::string features;
								for(unsigned int k=2;k<stem_entries->nr_of_result_rows();++k){
									feature=*stem_entries->field_value_at_row_position(k,"feature");
									features+="+"+feature;
								}
								strong_stems_ppart.insert(std::make_pair(stem,std::make_pair(wordform,features)));
							}
						}
					}
					if(feature_weak!=NULL){
						weak_stems.insert(stem);
						if(std::string("aeiouyw").find(stem.back())==std::string::npos&&wordform==stem+std::string(1,stem.back())+"ed"){//consonant doubling
							past_stems_dcon.insert(stem);
						}
					}
					if(feature_inf!=NULL){
						stems.insert(stem);
					}
					if(feature_pres!=NULL&&(wordform.back()!='s'||stem=="have"||stem=="do"||stem=="be")){
						//This is ugly but found no other way to include "is, has, does" and exclude
						//all other regular singular present forms like "eats, works, dreams, etc.".
						auto upper_bound=present_stems.upper_bound(stem);
						bool wordform_found=false;
						for(auto k=present_stems.lower_bound(stem);k!=upper_bound;++k){
							if(k->second.first==wordform){
								wordform_found=true;
								break;
							}
						}
						if(wordform_found==false){
							std::string feature;
							std::string features;
							for(unsigned int k=2;k<stem_entries->nr_of_result_rows();++k){
								feature=*stem_entries->field_value_at_row_position(k,"feature");
								features+="+"+feature;
							}
							present_stems.insert(std::make_pair(stem,std::make_pair(wordform,features)));
						}
					}
					if(feature_prog!=NULL&&std::string("aeiouyw").find(stem.back())==std::string::npos&&wordform==stem+std::string(1,stem.back())+"ing"){//consonant doubling
						prog_stems_dcon.insert(stem);
					}
					verb_stem_entry=stem_entries->value_for_field_name_found_after_row_position(verb_stem_entry->first,"feature","V");
				}
				delete stem_entries;
			}
		}
		delete verb_entries;
	}
	for(auto i:stems){
		bool is_weak=false;
		bool is_strong=false;
		if(weak_stems.find(i)!=weak_stems.end()&&past_stems_dcon.find(i)==past_stems_dcon.end()) is_weak=true;
		if(strong_stems_past.find(i)!=strong_stems_past.end()
			||strong_stems_ppart.find(i)!=strong_stems_ppart.end())	is_strong=true;
		if(is_weak==true&&is_strong==false){
			std::cout<<i<<" "<<lexicon<<";"<<std::endl;
		}
		else if(is_strong==true&&is_weak==false){
			std::cout<<i<<" "<<lexicon<<"_strong;"<<std::endl;
		}
		else if(is_strong==true&&is_weak==true){
			std::cout<<i<<" "<<lexicon<<";"<<std::endl;
		}
	}
	for(auto i:present_stems){
		std::cout<<i.first<<"[stem]+V"<<i.second.second<<":"<<i.second.first<<" #;"<<std::endl;
	}
	for(auto i:strong_stems_past){
		std::cout<<i.first<<"[stem]+V"<<i.second.second<<":"<<i.second.first<<" #;"<<std::endl;
	}
	for(auto i:strong_stems_ppart){
		std::cout<<i.first<<"[stem]+V"<<i.second.second<<":"<<i.second.first<<" #;"<<std::endl;
	}
	std::cout<<"\nConsonant doubling lexc:\n";
	for(auto i:stems){
		if(prog_stems_dcon.find(i)!=prog_stems_dcon.end()
			&&past_stems_dcon.find(i)!=past_stems_dcon.end()){
			std::cout<<i<<" "<<lexicon<<";"<<std::endl;
		}
	}
	sqlite->close();
	db_factory::delete_instance();
	sqlite=NULL;
	return 0;
}
