#include "observation.h"
#include <dirent.h>
#include <iostream>
#include <experimental/filesystem>
#include <stdlib.h>

using namespace obs;
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]) {

  if(argc != 5){
    std::cout << "USAGE: ./run_preprocess <input file directory> <source> <output type> <output file directory>\n" << std::endl;
    std::cout << "Possible sources: eumetsat, jma, wisconsin, noaa, kma, avhrr" << std::endl;
    std::cout << "Possible output types: prepbufr, text\n" << std::endl;
    return 0;
  }
  std::string curr_dir(argv[1]);
  std::string source_arg(argv[2]);
  std::string output_arg(argv[3]);
  std::string output_dir(argv[4]);
  
  Observation obs;
  FileSource source;
  OutputFormat format;

  //add '/' to the end of directories just in case
  curr_dir = curr_dir + "/";
  output_dir = output_dir + "/";

  if( source_arg.compare("eumetsat") == 0 ){
    source = FileSource::fromEUMETSAT;
  }else if( source_arg.compare("jma") == 0 ){
    source = FileSource::fromJMA;
  }else if( source_arg.compare("wisconsin") == 0  ){
    source = FileSource::fromWisconsin;
  }else if( source_arg.compare("noaa") == 0 ){
    source = FileSource::fromNOAA;
  }else if( source_arg.compare("kma") == 0 ){
    source = FileSource::fromKMA;
  }else if (source_arg.compare("avhrr") == 0){
    source = FileSource::fromAVHRR;
  }else{
    std::cout << "Error: source " << source_arg << "  not available" << std::endl;
    std::cout << "Possible sources: eumetsat, jma, wisconsin, noaa, kma" << std::endl; 
    return 0;
  } 

  if( output_arg.compare("prepbufr") == 0 ){
    format = OutputFormat::toPrepbufr;
  }else if( output_arg.compare("text") == 0 ){
    format = OutputFormat::toTextFile;
  }else{
    std::cout << "Error: output " << output_arg <<  " not available" << std::endl;
    std::cout << "Possible output types: prepbufr, text"  << std::endl;
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
        output_dirs = obs.evaluate_file(complete_filename, source, format, output_dir);

        if(!output_dirs.empty()){ 
          for( auto dir_name : output_dirs  ){
            //if not already in the output_dir_vect, add it
            if(std::find(output_dir_vect.begin(), output_dir_vect.end(), 
                   dir_name) == output_dir_vect.end()){
                 output_dir_vect.push_back(dir_name);
            }
          }

        }else{
          std::cout << "ERROR: no output directory returned." << std::endl;
        }

        std::cout << "Completed processing for " << entry->d_name << std::endl;
        
      }
    }
  }
  closedir(dp);
    
  for (auto & dir : output_dir_vect) {
    std::cout << dir << std::endl;
  
    if(output_arg.compare("prepbufr")==0){

      //get the directory string of the output_file
      fs::path p = dir;
      std::string parent_dir = dir; 
      std::string python_cmd = "python ";

      //pass parent directory to python script to finish off the files
      if( source_arg.compare("eumetsat") == 0 ){

        std::cout << "run eumetsat python scripts..." << std::endl;

        python_cmd += "src/python_scripts/eumetsat_final_process.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());

        python_cmd = "python src/python_scripts/eumetsat_fill_in_bufr.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());
      }
      if( source_arg.compare("jma") == 0 ){

        std::cout << "run jma python scripts..." << std::endl;

        python_cmd += "src/python_scripts/jma_final_process.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());
    
        python_cmd = "python src/python_scripts/jma_fill_in_bufr.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());
     
      }
      if( source_arg.compare("wisconsin") == 0  ){

        std::cout << "run wisconsin python scripts..." << std::endl;

        python_cmd += "src/python_scripts/goes_final_process.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());

        python_cmd = "python src/python_scripts/goes_fill_in_bufr.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());
      }
      
      if( source_arg.compare("noaa") == 0  ){

        std::cout << "run noaa python scripts..." << std::endl;

        python_cmd += "src/python_scripts/noaa_final_process.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());

        python_cmd = "python src/python_scripts/noaa_fill_in_bufr.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());
      }
      if( source_arg.compare("kma") == 0  ){

        std::cout << "run kma python scripts..." << std::endl;

        python_cmd += "src/python_scripts/kma_final_process.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());

        python_cmd = "python src/python_scripts/kma_fill_in_bufr.py ";
        python_cmd += parent_dir;
        system(python_cmd.c_str());
      }
    }
   std::cout << output_dirs.size() << std::endl;
  }
  return 0; 
}

