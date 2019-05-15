#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>
#include "logger.h"
#include "db_factory.h"
#include "query_result.h"
db *db_factory::singleton_instance=NULL;

bool is_space(std::string::value_type c){
	return std::isspace(c,std::locale());
}

int main(int argc, char* argv[]){

	db *sqlite=NULL;
	std::string db_file,morphtxt_file,morph_line;
	std::ifstream *filestream=NULL;

	if(argc<3){
		std::cerr<<"Usage: upenn_morphtxt2db /path/to/comment/free/morph/txt/file /path/to/dbfile.db"<<std::endl;
		exit(EXIT_FAILURE);
	}
	morphtxt_file=argv[1];
	db_file=argv[2];
	filestream=new std::ifstream(morphtxt_file);
	if(filestream==NULL){
		std::cerr<<"Error opening file "<<morphtxt_file<<std::endl;
		exit(EXIT_FAILURE);
	}
	sqlite=db_factory::get_instance();
	sqlite->open(db_file);
	while(std::getline(*filestream,morph_line)){
		unsigned int slice_nr=0;
		std::stringstream splithash(morph_line);
		std::string wordform,slice;
		while(std::getline(splithash,slice,'#')){
			if(slice_nr==0){
				auto space_pos=std::find_if(slice.begin(),slice.end(),is_space);
				wordform=slice.substr(0,std::distance(slice.begin(),space_pos));
				slice=slice.substr(std::distance(slice.begin(),space_pos));
			}
			//std::cout<<"wordform:"<<wordform;
			std::string rep_slice;
			for(auto i:slice){
				if(std::isspace(i,std::locale())==false){
					rep_slice+=i;
				}
				else{
					if(rep_slice.empty()==false&&rep_slice.back()!=' ') rep_slice+=' ';
				}
			}
			std::stringstream spacehash(rep_slice);
			std::string field,sql_values;
			unsigned int field_nr=0,record_nr=0;
			std::string rep_wordform;
			for(auto i:wordform){
				if(i!='\''){
					rep_wordform+=i;
				}
				else{
					rep_wordform+="''";
				}
			}
			while(std::getline(spacehash,field,' ')){
				std::string rep_field;
				for(auto i:field){
					if(i!='\''){
						rep_field+=i;
					}
					else{
						rep_field+="''";
					}
				}
				if(rep_wordform.empty()==false&&rep_field.empty()==false){
					sql_values="'"+rep_wordform+"','"+std::to_string(slice_nr)+"','"+std::to_string(record_nr)+"','"+rep_field+"'";
					//std::cout<<std::endl<<"Writing to db:"<<sql_values<<std::endl;
					sqlite->exec_sql("INSERT INTO MORPHOLOGY (WORDFORM,GROUP_ID,FEATURE_NR,FEATURE) VALUES ("+sql_values+");");
					++record_nr;
				}
				//std::cout<<" field"<<field_nr<<":"<<field;
				++field_nr;
			}
			//std::cout<<std::endl;
			++slice_nr;
		}
	}
	filestream->close();
	sqlite->close();
	db_factory::delete_instance();
	sqlite=NULL;
	return 0;
}
