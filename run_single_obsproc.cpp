#include "observation.h"
#include <dirent.h>
#include <iostream>
#include <experimental/filesystem>
#include <stdlib.h>

using namespace obs;
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]) {

  if(argc != 4){
    std::cout << "USAGE: ./test <input file> <source> <output type>\n" << std::endl;
    std::cout << "Possible sources: eumetsat, jma, wisconsin" << std::endl;
    std::cout << "Possible output types: prepbufr, text, hdf\n" << std::endl;
    return 0;
  }
  std::string curr_file(argv[1]);
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


  std::vector<std::string> output_dirs;
  std::vector<std::string> output_dir_vect;

  output_dirs = obs.evaluate_file(curr_file, source, format);
  
}

