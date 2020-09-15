//nrl_observation.cpp

#include "observation_implementation.h"
using namespace obs;

/* ----------------------------------------------------------------------------
NRL Observation Member Definitions
---------------------------------------------------------------------------- */

NRLObservation::NRLObservation(){};
NRLObservation::~NRLObservation(){};


void
NRLObservation::populate_metadata(const std::string& str){

  std::string ndate, tmp1, tmp2;
  std::istringstream iss(str);
  iss >> tmp1 >> std::skipws >> ndate >> std::skipws >>
  tmp2 >> std::skipws >> this->stationid;
  this->ndate = atoi(ndate.c_str());
}

void
NRLObservation::load_data(){
  std::string str;
  std::ifstream infile(this->filename.c_str());
  
  if(!infile.is_open())
    std::exit(EXIT_FAILURE);//make this consistent with the other exceptions

  std::getline(infile, str);
  populate_metadata(str);

  std::getline(infile, str);
  std::istringstream iss(str);
  std::vector<std::string> 
       parsed_labels(std::istream_iterator<std::string>(iss),{});
  int order_index;  
  //initialize map with an implicit order

  for(auto label : parsed_labels){
    this->data_storage.push_back(std::pair<std::string, 
         std::vector<std::string>>(label, std::vector<std::string>()));
  }
  //use order to input label

  //loading the data
  while(std::getline(infile, str)){
       
    std::istringstream iss(str);
    std::vector<std::string> 
       parsed_data(std::istream_iterator<std::string>(iss), {});  
    std::vector<std::string>::const_iterator data_iter = parsed_data.begin();

    //additional processing because of some white spaces
    if(parsed_data.size() != parsed_labels.size()){
      std::vector<std::string> tmp;
      while(parsed_data.size() >= parsed_labels.size()){
         tmp.push_back(parsed_data.back());
         parsed_data.pop_back();
      }
      std::string tmp_str = "";      
      for(auto element : tmp){
        if(element.compare("--")){
         element.erase(remove(element.begin(), element.end(), '-'), element.end());
         tmp_str = tmp_str + element;
        }
      }
      parsed_data.push_back(tmp_str);
    }
   
    order_index = 0;
    for(auto data_field:parsed_data){
      this->data_storage.at(order_index).second.push_back(data_field);
      ++order_index;
    }
    ++this->obs_count;
  }

  //create hour window field
  std::string curr_hour_window = std::to_string(this->ndate).substr(8);
  std::vector<std::string> hour_window(this->obs_count, curr_hour_window);
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("hour_window", hour_window));
}

/*
void
NRLObservation::set_additional_fields(){

  std::vector<std::string> wind_qcflag;
  std::vector<std::string> pres_qcflag;
  std::vector<std::string> pres_error;
  std::vector<std::string> wind_error;

  if(this->obs_count == 0){
    throw ZeroCount();
  }
  wind_qcflag.assign(this->obs_count, "2");
  pres_qcflag.assign(this->obs_count, "2");
  pres_error.assign(this->obs_count, MISS_STR);
  wind_error.assign(this->obs_count, MISS_STR);
  
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("wqcflag", wind_qcflag));
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("pqcflag", pres_qcflag));
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("pres_err", pres_error));
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("wind_err", wind_error));
}
*/

void
NRLObservation::import_file(){

  load_data();
  //set_additional_fields();
}

void
NRLObservation::preprocess(){
  import_file();
}


