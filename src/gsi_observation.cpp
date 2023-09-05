//gsi_observation.cpp

#include "observation_implementation.h"
using namespace obs;

/* ----------------------------------------------------------------------------
GSI Observation Member Definitions
---------------------------------------------------------------------------- */

GSIObservation::GSIObservation(){};
GSIObservation::~GSIObservation(){};

void
GSIObservation::load_data(){

  this->stationid = "GSINETCF";
  try{
    netCDF::NcFile file(this->filename, netCDF::NcFile::read);
    
    int date_time[1];
    netCDF::NcGroupAtt attrs = file.getAtt("date_time");
    attrs.getValues(date_time);
    this->ndate = (long) date_time[0];
    
    netCDF::NcDim nc_nobs = file.getDim("nobs");
    int nobs = nc_nobs.getSize();
    this->obs_count = nobs;
    int var_count = file.getVarCount();
    
    std::multimap<std::string, netCDF::NcVar> ncvars= file.getVars();
    int ncvars_size = ncvars.size();
   
    //convert every single field to string and save to map     
    int int_arr[nobs];
    std::string str_arr[nobs];
    double double_arr[nobs];
    
    std::vector<std::string> str_vect;
    for(auto var : ncvars){
      netCDF::NcType nctype = var.second.getType();

      str_vect.clear();
      if(!(nctype.getName()).compare("float")){
        var.second.getVar(double_arr);
        for(auto element : double_arr){
          str_vect.push_back(std::to_string(element));
        }
      } else if(!(nctype.getName()).compare("int")){
        var.second.getVar(int_arr);
        for(auto element : int_arr){
          str_vect.push_back(std::to_string(element));
        }
      } else if(!(nctype.getName()).compare("char")){
        netCDF::NcDim nc_char_dim = file.getDim((var.first + "_maxstrlen"));
        int char_dim = nc_char_dim.getSize();
        char char_arr[(nobs * char_dim) + char_dim];
        var.second.getVar(char_arr);

        int count = 0;
        std::string str = "";
        for(auto row : char_arr){
          if((count % char_dim == 0) && (count > 0)){
            str_vect.push_back(str);
            str = "";
            ++count;
          }else {
           ++count;
          }
          str = str + row;
        }
        
      } else{
        std::cout << "Can't read in " << nctype.getName() << std::endl;
        throw TypeUnavailable();
      } 
      this->data_storage.push_back(std::pair<std::string, 
                std::vector<std::string>>(var.first, str_vect));   
    }

    std::string curr_hour_window = std::to_string(date_time[0]).substr(8);
    std::vector<std::string> hour_window(this->obs_count, curr_hour_window);
    this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("hour_window", hour_window));
    
  }catch(netCDF::exceptions::NcException e){
    e.what();
    std::cout << "Failure to read ncdiag file" << std::endl;
    return;
  }
}

void
GSIObservation::import_file(){
  load_data();
}

void
GSIObservation::preprocess(){
  import_file();
}

