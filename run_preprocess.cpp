#include "observation.h"
#include <dirent.h>
#include <iostream>
#include <experimental/filesystem>
#include <stdlib.h>

using namespace obs;
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]) {

  if(argc != 4){
    std::cout << "USAGE: ./test <input file directory> <source> <output type>\n" << std::endl;
    std::cout << "Possible sources: eumetsat, jma, wisconsin" << std::endl;
    std::cout << "Possible output types: prepbufr, text, hdf\n" << std::endl;
    return 0;
  }
  std::string curr_dir(argv[1]);
  std::string source_arg(argv[2]);
  std::string output_arg(argv[3]);

  
  Observation obs;
  FileSource source;
  OutputFormat format;

  if( source_arg.compare("eumetsat") == 0 ){
    source = FileSource::fromEUMETSAT;
  }else if( source_arg.compare("jma") == 0 ){
    source = FileSource::fromJMA;
  }else if( source_arg.compare("wisconsin") == 0  ){
    source = FileSource::fromWisconsin;
  }else{
    std::cout << "Error: source not available" << std::endl;
    std::cout << "Possible sources: eumetsat, jma, wisconsin" << std::endl; 
    return 0;
  } 


  if( output_arg.compare("prepbufr") == 0 ){
    format = OutputFormat::toPrepbufr;
  }else if( output_arg.compare("text") == 0 ){
    format = OutputFormat::toTextFile;
  }else if( output_arg.compare("hdf") == 0  ){
    format = OutputFormat::toHdf;
  }else{
    std::cout << "Error: output not available" << std::endl;
    std::cout << "Possible output types: prepbufr, text, hdf"  << std::endl;
    return 0;
  } 

  
  std::string complete_filename;
  std::string filename;
  std::vector<std::string> output_dirs;
  std::vector<std::string> output_dir_vect;

  struct dirent *entry = nullptr;
  DIR *dp = nullptr;
  dp = opendir(curr_dir.c_str());
  int count = 0;
  if(dp != nullptr){
    while ((entry = readdir(dp))){
      filename = entry->d_name;
      complete_filename = curr_dir + filename;
      if((filename.compare(".")) && (filename.compare(".."))){
        std::cout << complete_filename << std::endl;
        output_dirs = obs.evaluate_file(complete_filename, source, format);
        //append all output directories to a new vector if it doesn't exist
        //std::cout << output_dirs.front() << std::endl;
        if(output_dirs.size() == 1){ 
          if( (std::find(output_dir_vect.begin(), output_dir_vect.end(), output_dirs.front()) == output_dir_vect.end()) || output_dir_vect.empty() ) {
            output_dir_vect.push_back(output_dirs.front());
          }
        }
        std::cout << "Completed wind processing for " << entry->d_name << std::endl;
        //break;
      }
    }
  }
  closedir(dp); 
  //return 0;  
  for (auto & dir : output_dir_vect) {
    std::cout << dir << std::endl;
  
    if(output_arg.compare("prepbufr")==0){

      //get the directory string of the output_file
      //fs::path p = dir;
      //std::cout << p.parent_path() << std::endl;
      //std::string parent_dir = p.parent_path().string();  
      std::string parent_dir = dir; 

      std::string python_cmd = "python ";

      //pass parent directory to python script to finish off the files
      if( source_arg.compare("eumetsat") == 0 ){
        python_cmd += "eumetsat_final_process.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());
      }
      if( source_arg.compare("jma") == 0 ){
        python_cmd += "jma_final_process.py ";
        python_cmd += parent_dir;
        //std::cout << python_cmd << std::endl;
        system(python_cmd.c_str());
    
        python_cmd = "python jma_fill_in_bufr.py ";
        python_cmd += parent_dir;
        //python_cmd += "../intermediate_bufr/";
        system(python_cmd.c_str());
     
      }
      if( source_arg.compare("wisconsin") == 0  ){
        python_cmd += "goes_final_process.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());
      }
    }
   //break; 
  }

}

