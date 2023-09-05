//kma_observation.cpp

#include "observation_implementation.h"
#include <ISO_Fortran_binding.h>
using namespace obs;

extern "C" {

  void data_kma(double (**data_ptr)[14], int ** int_ptr, const char (**date_ptr),
            const char (**file_ptr), int * str_length);
  void get_labels_kma(char(**str));
}

/* ----------------------------------------------------------------------------
KMA Observation Member Definitions
---------------------------------------------------------------------------- */

KMAObservation::KMAObservation(){};
KMAObservation::~KMAObservation(){};

void
KMAObservation::import_file(){
  
  this->stationid = "KMASATWD";
  char str[200] = "";
  char * str_ptr = str; 

  int obs_count = 1;
  int *int_ptr = &obs_count;
 
  //break down KMA filename to retrieve datetime - change later  
  std::stringstream ss(this->filename);
  std::vector<std::string> fname_tokens;
  std::string token;
  while(std::getline(ss, token, '_')){
    fname_tokens.push_back(token);
  }

  std::string date = fname_tokens.at(2).substr(0, 10);
  std::string hour_str = fname_tokens.at(2).substr(8, 2);   

  const char * date_ptr = date.c_str();
  this->ndate = atol(date.c_str());
    
  int str_length = std::strlen(this->filename.c_str());
  const char * file_ptr = this->filename.c_str();
  //get labels
  get_labels_kma(&str_ptr);
  std::istringstream iss(str);
  std::vector<std::string> 
       parsed_labels(std::istream_iterator<std::string>(iss),{});
  std::cout << "cleared get labels" << std::endl;
  //change label to old_min
  for(auto label : parsed_labels){
    if((label.find("time") != std::string::npos)){
      this->data_storage.push_back(std::pair<std::string, 
           std::vector<std::string>>("old_min", std::vector<std::string>()));
    }else{
      this->data_storage.push_back(std::pair<std::string, 
           std::vector<std::string>>(label, std::vector<std::string>()));
    }
  }
  int column_size = parsed_labels.size();
  double data_arr[500000][column_size];
  double (*data_ptr)[column_size] = data_arr;
  
  //get data and load it into memory
  data_kma(&data_ptr, &int_ptr, &date_ptr, &file_ptr, &str_length);

  //std::cout << "here?" << std::endl;
  for(int i = 0; i < obs_count; i++){
    for(int column = 0; column < column_size; column++){
      this->data_storage.at(column).second
           .push_back(std::to_string(data_arr[i][column]));
    } 
  }

  double int_hour = std::stod(hour_str);
  std::string final_hour_window;

  if( (int_hour <= 3)||(int_hour > 21) ){
     final_hour_window = "00";
  }
  if( (int_hour > 3)&&(int_hour <= 9) ){
     final_hour_window = "06";
  }
  if( (int_hour > 9)&&(int_hour <= 15) ){
     final_hour_window = "12";
  }
  if( (int_hour > 15)&&(int_hour <= 21) ){
     final_hour_window = "18";
  }

  double hh, mm, new_time;
  hh = int_hour;

  if( int_hour > 21){
    hh = hh - 24;
  }else{
    hh = hh - std::stod(final_hour_window);
  }
  mm = 0; //not needed
  new_time = hh + mm;  

  //populate new time to go with new hour window
  std::vector<std::string> time(obs_count, std::to_string(new_time));
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("time", time));
  
  //generate hour window
  std::vector<std::string> hour_window(obs_count, final_hour_window);
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("hour_window", hour_window));

  this->obs_count = obs_count;

  //update ndate to have corrected hour window
  std::string temp_ndate = std::to_string(this->ndate);
  temp_ndate = temp_ndate.substr(0, 8);
  //update ndate in case it's the next day
  //pull out the first value for both time and hour window

  std::string ndate_time;
  std::string ndate_hour_window;
  std::string corrected_date;

  for(auto const& x : this->data_storage){
    if( (x.first.find("time") != std::string::npos)  ){
      ndate_time = x.second.front();
    }
  }

  for(auto const& x : this->data_storage){
    if( (x.first.find("hour_window") != std::string::npos)  ){
      ndate_hour_window = x.second.front();
    }
  }
  corrected_date = update_date(temp_ndate, ndate_time, ndate_hour_window);
  corrected_date = corrected_date + final_hour_window;
  this->ndate = std::stol(corrected_date);
}

void
KMAObservation::preprocess(){
  import_file();
}

