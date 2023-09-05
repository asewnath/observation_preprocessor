//jma_observation.cpp

#include "observation_implementation.h"
#include <boost/algorithm/string.hpp>
#include <cmath> 
#include <math.h>

using namespace obs;

JMAObservation::JMAObservation(){};
JMAObservation::~JMAObservation(){};

/*
variables required:
-lat
-lon
-time
-pres
-uwnd
-vwnd
-obstype
  -ir  :252
  -vis :242
  -wv  :250
-satid :171
-hour window
*/

const std::string JMA_WV  = "250";
const std::string JMA_VIS = "242";
const std::string JMA_IR  = "252";
const std::string JMA_SATID = "171";


void
JMAObservation::preprocess(){

  this->stationid = "JMAAMV";
  std::string str;
  std::ifstream infile(this->filename.c_str());
  std::getline(infile, str);

  std::vector<std::string> parsed_labels;
  boost::split(parsed_labels, str, [](char c){return c == ',';});

  //get labels
  for(auto label : parsed_labels){
    if((label.find("time") != std::string::npos) ||
                     (label.find("TIME") != std::string::npos) ){
      this->data_storage.push_back(std::pair<std::string, 
           std::vector<std::string>>("mod_jul", std::vector<std::string>()));
    }else{
      this->data_storage.push_back(std::pair<std::string, 
           std::vector<std::string>>(label, std::vector<std::string>()));
    }
  }

  //get data
  int order_index;
  std::vector<std::string> parsed_data;
  while(std::getline(infile, str)){
    parsed_data.clear();
    boost::split(parsed_data, str, [](char c){return c == ',';});
    
    order_index = 0;
    for(auto data_field : parsed_data){
     this->data_storage.at(order_index).second.push_back(data_field);
     ++order_index;
    }
   ++this->obs_count;
  } 

  //find obstype
  std::stringstream ss(this->filename);
  std::vector<std::string> fname_tokens;
  std::string token;
  while(std::getline(ss, token, '/')){
    fname_tokens.push_back(token);
  }  
  std::vector<std::string> fname_parts;
  boost::split(fname_parts, fname_tokens.back(), [](char c){return c == '_';});


  std::string str_obstype = fname_parts.back().substr(0, fname_parts.back().find('.'));
  std::string str_int_obstype;
  if(!str_obstype.compare("wv")){
    str_int_obstype = JMA_WV;
  }else if(!str_obstype.compare("vis")){
    str_int_obstype = JMA_VIS;
  }else if(!str_obstype.compare("ir")){
    str_int_obstype = JMA_IR; 
  }else{
    //throw error
    throw DataNotFound();
  }
  
  std::cout << "obstype is " << str_obstype << " which sets to " << str_int_obstype;
  std::vector<std::string> obstype(this->obs_count, str_int_obstype);
  this->data_storage.push_back(std::pair<std::string, 
                  std::vector<std::string>>("obstype", obstype));

  //find satid
  std::vector<std::string> satid(this->obs_count, JMA_SATID);
  this->data_storage.push_back(std::pair<std::string,
                   std::vector<std::string>>("satid", satid));

  //find time and hour window
  //retrieve time vector
  std::vector<std::string> time;
  for(auto const& x : this->data_storage){
    if( (x.first.find("mod_jul") != std::string::npos) ){
      time = x.second; 
    }
  }
  if(time.empty())
    throw DataNotFound();

  double hour, min, hh, new_time;
  int int_hour;
  std::string final_hour_window = fname_parts.at(1).substr(8, 2);
  std::vector<std::string> time_vect;

  for(auto element : time){

    element = element.substr((element.find('.')));
    hour = std::stod(element) * 24; //global
    hh = floor(hour);
    min = remainder(hour, 1);

    if(hour > 21){
      hh = hh - 24;
    }else{
      hh = hh - std::stod(final_hour_window);
    }

    new_time = hh + min;
    time_vect.push_back(std::to_string(new_time));
  }

  this->data_storage.push_back(std::pair<std::string, 
                  std::vector<std::string>>("time", time_vect));

  //remove time and called it old_mjd

  //set hour window
  std::vector<std::string> hour_window(this->obs_count, final_hour_window); //global
  this->data_storage.push_back(std::pair<std::string,
                   std::vector<std::string>>("hour_window", hour_window));

  //set date
  std::string temp_ndate;
  temp_ndate = fname_parts.at(1).substr(0, 8);
  final_hour_window = fname_parts.at(1).substr(8, 2);
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
  
  this->ndate = std::stol(temp_ndate+final_hour_window);

  //find 'QI not using NWP' and set to qifn and save to data structure
  std::vector<std::string> qifn; 
  for(auto const& x : this->data_storage){
    if( (x.first.find("QI not using NWP") != std::string::npos) ){
      qifn = x.second;
      this->data_storage.push_back(std::pair<std::string,
                   std::vector<std::string>>("qifn", qifn));
    }
  }
  //find 'QI using NWP' and set to qify and save to data structure
  std::vector<std::string> qify;
  for(auto const& x : this->data_storage){
    if( (x.first.find("QI using NWP") != std::string::npos) ){
      qify = x.second;
      this->data_storage.push_back(std::pair<std::string,
                   std::vector<std::string>>("qify", qify));
    }
  }

  //correct negative lon values
  std::vector<std::string> org_lon;
  std::string str_lon;
  for(auto const& x : this->data_storage){
    if( (x.first.find("lon") != std::string::npos) ||
                     (x.first.find("LON") != std::string::npos) ){
      str_lon = x.first;
      org_lon = x.second;
    }
  }

  if(org_lon.empty())
    throw DataNotFound();

  double tmp_lon;
  std::vector<std::string> lon;
  for(auto element : org_lon){ 
    tmp_lon = stod(element) * -1;
    if(tmp_lon < 0){
      //prepbufr processing expects non-negative values
      tmp_lon = tmp_lon + 360; 
    }   
    //data check after correction
    if( (tmp_lon < 0) || (tmp_lon > 360) ){
      throw DataNotFound();
    }
    lon.push_back(std::to_string(tmp_lon)); 
  } 
  std::vector<std::pair<std::string,std::vector<std::string>>>::iterator ptr;
  for(ptr=this->data_storage.begin(); ptr < this->data_storage.end(); ptr++){
    if(ptr->first == str_lon){
      this->data_storage.erase(ptr);
    }
  } 
  this->data_storage.push_back(std::pair<std::string,
                    std::vector<std::string>>("lon", lon));



}
