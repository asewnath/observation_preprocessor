//observation_functions.cpp

#include "observation_functions.h"

void
skip_line(std::ifstream& infile){
  std::string str;
  std::getline(infile,str);
}

std::string&
remove_quotes(std::string & str){
  str.erase(remove(str.begin(), str.end(), '\"'), str.end());
  return str;
}

std::string&
retrieve_name(std::string& str){
  remove_quotes(str);
  return str;
}

std::string
get_filename(const std::string& str){
  char delimiter = '/';
#ifdef _WIN32
  delimiter = '\\';
#endif

  //check if delimiter exists
  if( std::find(str.begin(), str.end(), delimiter) == str.end() ){
    return str;
  } 
    
  size_t pos = str.rfind(delimiter, str.length());
  if (pos != std::string::npos){
    return(str.substr(pos+1, str.length() - pos));
  }else{
    return "";
  }

}
