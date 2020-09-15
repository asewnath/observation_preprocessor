//testbench.cpp
#include "observation.h"
#include <dirent.h>
#include <iostream>

using namespace obs;

int main(int argc, char *argv[]) {
  
  if(argc != 2){
    std::cout << "USAGE: ./test <input file directory>";
  }
  //curr_dir = argv[0]; 
  std::string curr_dir(argv[1]);  
  //std::cout << curr_dir << std::endl;

  //std::ofstream LOGFILE;
  //remove("obs_preprocessor_logfile.txt"); //check if it exists first
  //LOGFILE.open("obs_preprocessor_logfile.txt");
  //LOGFILE << "satellite" << "\t" << "datetime_hour"
  //    << "\t" << "start_time" << "\t" << "end_time"
  //    << std::endl;
  //LOGFILE.close();  
  
  
  //this will be a lower level function call
    
  Observation obs;
  FileSource source;
  OutputFormat format;

  std::string complete_filename;
  std::string filename;
  source = FileSource::fromWisconsin;
  //source = FileSource::fromEUMETSAT;
  //source = FileSource::fromJMA;
  //format = OutputFormat::toTextFile;
  format = OutputFormat::toPrepbufr;
  struct dirent *entry = nullptr;
  DIR *dp = nullptr;
  //dp = opendir("/discover/nobackup/asewnath/GEOSadas/src/GMAO_Shared/GMAO_ncdiag/tmpdir/uw_2004");
  //dp = opendir("/discover/nobackup/asewnath/GEOSadas/src/GMAO_Shared/GMAO_ncdiag/tmpdir/observation_preprocessor/input/eumetsat/eumetsat_data");
  //dp = opendir("/discover/nobackup/asewnath/GEOSadas/src/GMAO_Shared/GMAO_ncdiag/tmpdir/observation_preprocessor/input/JMA/Reprocessed_AMV_JMA_MSC/Himawari8_algorithm/CSV");
  //dp = opendir("/discover/nobackup/asewnath/GEOSadas/src/GMAO_Shared/GMAO_ncdiag/tmpdir/observation_preprocessor/input/GOES/aug_2005/org_data");
  //std::string curr_dir = "../input/GOES/aug_2005/org_data/";
  dp = opendir(curr_dir.c_str()); 
  int count = 0;
  if(dp != nullptr){
    while ((entry = readdir(dp))){ 
      filename = entry->d_name;
      complete_filename = curr_dir + filename;
      if((filename.compare(".")) && (filename.compare(".."))){
        std::cout << complete_filename << std::endl;
        obs.evaluate_file(complete_filename, source, format);
        std::cout << "Completed wind processing for " << entry->d_name << std::endl;
      }
    }
  }
  closedir(dp);
  
  
  /* 
  Observation obs;
  FileSource source;
  OutputFormat format;

  std::vector<std::string> output_filenames;

  std::string jma_filename = "../input/JMA/Reprocessed_AMV_JMA_MSC/Himawari8_algorithm/CSV/amv_2005061600_f_wv.CSV";
  source = FileSource::fromJMA;
  format = OutputFormat::toTextFile;
  //format = OutputFormat::toPrepbufr;
  output_filenames = obs.evaluate_file(jma_filename, source, format);
  std::cout << "JMA" << std::endl;
  */

  /*
  std::string metsat_filename = "./input/MSG1-SEVI-MSGAMVE0100-0100-0100-20040801024500.000000000Z-20140510195926-1356738.bfr";
  source = FileSource::fromEUMETSAT;
  format = OutputFormat::toTextFile;
  output_filenames = obs.evaluate_file(metsat_filename, source, format);
  std::cout << "metsat/text filename output vector size: " << 
                output_filenames.size() << std::endl;

  output_filenames.clear();
  source = FileSource::fromEUMETSAT;
  format = OutputFormat::toPrepbufr;
  output_filenames = obs.evaluate_file(metsat_filename, source, format);
  std::cout << "metsat/pbufr filename output vector size: " << 
                output_filenames.size() << std::endl;

  output_filenames.clear();
  source = FileSource::fromEUMETSAT;
  format = OutputFormat::toTextFile;
  output_filenames = obs.evaluate_file(metsat_filename, source, format);
  std::cout << "metsat/text filename output vector size: " << 
                output_filenames.size() << std::endl;

  */

  /*
  output_filenames.clear();
  std::string nrl_filename = "./input/output.txt";
  source = FileSource::fromNRL;
  format = OutputFormat::toTextFile;
  output_filenames = obs.evaluate_file(nrl_filename, source, format);
  std::cout << "nrl/text filename output vector size: " << 
                output_filenames.size() << std::endl;

  output_filenames.clear();
  source = FileSource::fromNRL;
  format = OutputFormat::toPrepbufr;
  output_filenames = obs.evaluate_file(nrl_filename, source, format);
  std::cout << "nrl/pbufr filename output vector size: " << 
                output_filenames.size() << std::endl;

  output_filenames.clear();
  std::string uw_filename = "./input/_____-__-20181210-09";
  source = FileSource::fromWisconsin;
  format = OutputFormat::toTextFile;
  output_filenames = obs.evaluate_file(uw_filename, source, format);
  std::cout << "uw/text filename output vector size: " << 
                output_filenames.size() << std::endl;

  output_filenames.clear();
  source = FileSource::fromWisconsin;
  format = OutputFormat::toPrepbufr;
  output_filenames = obs.evaluate_file(uw_filename, source, format);
  std::cout << "uw/pbufr filename output vector size: " << 
                output_filenames.size() << std::endl;

  output_filenames.clear();
  uw_filename = "./input/GOESW-SH-20040818-08";
  source = FileSource::fromWisconsin;
  format = OutputFormat::toTextFile;
  output_filenames = obs.evaluate_file(uw_filename, source, format);
  std::cout << "uw/text filename output vector size: " << 
                output_filenames.size() << std::endl;

  output_filenames.clear();
  source = FileSource::fromWisconsin;
  format = OutputFormat::toPrepbufr;
  output_filenames = obs.evaluate_file(uw_filename, source, format);
  std::cout << "uw/pbufr filename output vector size: " << 
                output_filenames.size() << std::endl;

  output_filenames.clear();
  std::string gsi_filename = 
              "./input/x35_nothin.diag_conv_uv_ges.20180621_00z.nc4";
  source = FileSource::fromGSI;
  format = OutputFormat::toTextFile;
  output_filenames = obs.evaluate_file(gsi_filename, source, format);
  std::cout << "gsi/text filename output vector size: " << 
                output_filenames.size() << std::endl;

  output_filenames.clear();
  source = FileSource::fromGSI;
  format = OutputFormat::toPrepbufr;
  output_filenames = obs.evaluate_file(gsi_filename, source, format);
  std::cout << "gsi/pbufr filename output vector size: " << 
                output_filenames.size() << std::endl;
  */
  
}

