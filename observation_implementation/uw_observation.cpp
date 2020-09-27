//uw_observation.cpp

#include "observation_implementation.h"
using namespace obs;

/* ----------------------------------------------------------------------------
Wisconsin Observation Member Definitions
---------------------------------------------------------------------------- */

UWObservation::UWObservation(){};
UWObservation::~UWObservation(){};

void
UWObservation::load_data(){

  this->stationid = "WISCNAMV";
  std::string str;
  std::ifstream infile(this->filename.c_str());
  std::getline(infile, str);
  std::istringstream iss(str);
  std::vector<std::string> 
       parsed_labels(std::istream_iterator<std::string>(iss),{});
  
  for(auto label : parsed_labels){
    this->data_storage.push_back(std::pair<std::string,
         std::vector<std::string>>(label, std::vector<std::string>()));
  } 
 
 int order_index;  
  while(std::getline(infile, str)){
    std::istringstream iss(str);
    std::vector<std::string>
       parsed_data(std::istream_iterator<std::string>(iss), {});

    order_index = 0;
    for(auto data_field : parsed_data){
      this->data_storage.at(order_index).second.push_back(data_field);
      ++order_index;
    }   
    ++this->obs_count;
  }
  //std::cout << "done" << std::endl;
}

void
UWObservation::set_date(const std::string& hour_window1, 
                         const std::string& hour_window2){
  std::string date;
  std::string org_date;
  std::string day_label;
  
  date = get_filename(this->filename);
  date = date.substr(9, 8);

  this->ndate = stol((date + hour_window1 + hour_window2));

  for(auto const& x : this->data_storage){
    if( (x.first.find("day") != std::string::npos) ||
                     (x.first.find("DAY") != std::string::npos) ){
      org_date = x.second.front(); 
      day_label = x.first;
    }
  }
  //Correction for if filename is different than day
  if(date.compare(org_date)){

    //this->data_storage.erase(day_label);
    std::vector<std::pair<std::string,std::vector<std::string>>>::iterator ptr;
    for(ptr=this->data_storage.begin(); ptr < this->data_storage.end(); ptr++){
      if(ptr->first == day_label){
        this->data_storage.erase(ptr);
      }
    }
    
    std::vector<std::string> new_day(this->obs_count, date);
    this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("day", new_day));  
  } 
}

void
UWObservation::time_conversion(){
  //prepare time into a format that analysis accepts and create ndate
  std::vector<std::string> hms;
  for(auto const& x : this->data_storage){
    if( (x.first.find("hms") != std::string::npos) ||
                     (x.first.find("HMS") != std::string::npos) ){
      hms = x.second; 
    }
  }
  if(hms.empty())
    throw DataNotFound();

  //find start time and end time
  double start_time, end_time;
  std::tie(start_time, end_time) = find_time_range(hms);

  //do range determinations
  std::string str_hour_window1, str_hour_window2;
  std::tie(str_hour_window1, str_hour_window2) = 
               determine_hour_windows(start_time, end_time);

  double curr_hour_window, new_time;
  std::vector<std::string> time, hour_window;
  for(auto& old_time : hms){
    //calculate deviations function
    std::tie(new_time, curr_hour_window) = calculate_time(old_time);
    time.push_back(std::to_string(new_time));

    //std::cout << "str_hour_window: " << str_hour_window1  <<std::endl;
    //std::cout << "curr_hour_window: " << curr_hour_window << std::endl;

    if(stoi(str_hour_window1) == curr_hour_window){
      hour_window.push_back(str_hour_window1);
    }else{
      hour_window.push_back(str_hour_window2);
    }
  }
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("time", time));
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("hour_window", hour_window));
  
  set_date(str_hour_window1, str_hour_window2);
}


void
UWObservation::uv_conversion(){
  //find speed and direction and convert to u and v
  std::vector<std::string> spd;
  for(auto const& x : this->data_storage){
    if( (x.first.find("spd") != std::string::npos) ||
                     (x.first.find("SPD") != std::string::npos) ){
      spd = x.second; 
    }
  }
  if(spd.empty())
    throw DataNotFound();
 
  std::vector<std::string> dir;
  for(auto const& x : this->data_storage){
    if( (x.first.find("dir") != std::string::npos) ||
                     (x.first.find("DIR") != std::string::npos) ){
      dir = x.second; 
    }
  } 
  if(dir.empty())
    throw DataNotFound();

  std::vector<std::string> uwnd, vwnd;
  std::tie(uwnd, vwnd) = convert_to_uv(spd, dir);

  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("uwnd", uwnd));
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("vwnd", vwnd));
}

std::string
set_obstype( json & sat, std::string& type ){
  std::string obs_type_str;
  if((type.compare("IR") == 0)||(type.compare("ir") == 0)){
    obs_type_str = sat["ir"];
  }else if((type.compare("SWIR") == 0)||(type.compare("swir") == 0)){
    obs_type_str = sat["swir"];
  }else if((type.compare("WV") == 0)||(type.compare("wv") == 0)){
    obs_type_str = sat["wv"];
  }else if ((type.compare("WVCS") == 0)||(type.compare("wvcs") == 0)){
    obs_type_str = sat["wvcs"];
  }else if((type.compare("WVCT") == 0)||(type.compare("wvct") == 0)){
    obs_type_str = sat["wvct"];
  }else if((type.compare("WVCA") == 0)||(type.compare("wvca") == 0)){
    obs_type_str = sat["wvca"];
  }else if((type.compare("VIS") == 0)||(type.compare("vis") == 0)){
    obs_type_str = sat["vis"];
  }else{
    throw MissingObsType();
  }
  return remove_quotes(obs_type_str);
}

std::string
set_subtype( json & sat ){
  std::string sat_id_str;
  sat_id_str = sat["satid"];
  return remove_quotes(sat_id_str);
}

std::tuple<std::vector<std::string>, std::vector<std::string>>
retrieve_ids(std::vector<std::string>& sat, std::vector<std::string>& type){

  std::ifstream sat_stream(JSON_SATLIST);
  json sat_list = json::parse(sat_stream);
  std::string str, sat_name;

  std::vector<std::string> subtype;
  std::vector<std::string> obstype;
  for (auto tuple : boost::combine(sat, type)){
    std::string sat, type;
    boost::tie(sat, type) = tuple;
    
    for(int i = 0; i < sat_list["satellites"].size(); ++i){
      str = sat_list["satellites"][i]["name"];
      sat_name = remove_quotes(str);
      if(sat.compare(sat_name) == 0){
        subtype.push_back(set_subtype(sat_list["satellites"][i]));
        obstype.push_back(set_obstype(sat_list["satellites"][i], type));
        break;
      }
    }
  }
  return make_tuple(subtype, obstype);
}

void
UWObservation::find_numerical_id(){
  //find sat and type and convert to kx/kt
  std::vector<std::string> sat;
  for(auto const& x : this->data_storage){
    if( (x.first.find("sat") != std::string::npos) ||
                     (x.first.find("SAT") != std::string::npos) ){
      sat = x.second; 
    }
  }
  if(sat.empty())
    throw DataNotFound();

  std::vector<std::string> type;
  for(auto const& x : this->data_storage){
    if( (x.first.find("type") != std::string::npos) ||
                     (x.first.find("TYPE") != std::string::npos) ){
      type = x.second; 
    }
  }
  if(type.empty())
    throw DataNotFound();

  std::vector<std::string> subtype, obstype;
  std::tie(subtype, obstype) = retrieve_ids(sat, type);

  if(subtype.empty())
    throw MissingSubType();

  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("satid", subtype));
  this->data_storage.push_back(std::pair<std::string, 
                    std::vector<std::string>>("obstype", obstype));
}

void
UWObservation::lon_conversion(){

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

  std::vector<std::string> lon;
  for(auto element : org_lon){
    lon.push_back(std::to_string(stod(element) * -1));
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

void
UWObservation::setting_qi(){
  //find 'qinf' and set to qifn and save to data structure
  std::vector<std::string> qifn;
  for(auto const& x : this->data_storage){
    if( (x.first.find("qinf") != std::string::npos) ){
      qifn = x.second;
      this->data_storage.push_back(std::pair<std::string,
                   std::vector<std::string>>("qifn", qifn));
    }
  }
  //find 'qiwf' and set to qify and save to data structure
  std::vector<std::string> qify;
  for(auto const& x : this->data_storage){
    if( (x.first.find("qiwf") != std::string::npos) ){
      qify = x.second;
      this->data_storage.push_back(std::pair<std::string,
                   std::vector<std::string>>("qify", qify));
    }
  }
}


void
UWObservation::do_conversions(){
  time_conversion();
  uv_conversion();
  find_numerical_id();
  lon_conversion();
  setting_qi();
}

/*
void
UWObservation::set_additional_fields(){

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
UWObservation::import_file(){
  load_data();
  do_conversions();
  //set_additional_fields();
}

void
UWObservation::preprocess(){
  import_file();
}


/* ----------------------------------------------------------------------------
Wisconsin Observation Non-Member Definitions
---------------------------------------------------------------------------- */

std::tuple<double, double>
calculate_time(const std::string& old_time){
  //calculate deviations from hour windows
  double hhmm, hh, mm, new_time;
  std::string str_hh, str_mm, str_new_time;
  hhmm = stod(old_time);
  str_hh = old_time.substr(0, 2);
  str_mm = old_time.substr(2, 2);
  hh = stod(str_hh);
  mm = stod(str_mm);
    
  //determine which hour window contains hhmm
  double curr_hour_window;
  if( ((hhmm <= 300)||(hhmm > 2100)) ){
    curr_hour_window = 0;
  }else if( ((hhmm > 300)&&(hhmm <= 900)) ){
    curr_hour_window = 6;
  }else if( ((hhmm > 900)&&(hhmm <= 1500)) ){
    curr_hour_window = 12;
  }else if( ((hhmm > 1500)&&(hhmm <= 2100)) ){
    curr_hour_window = 18;
  }else{
    throw OutOfBounds();
  }

  if(hhmm > 2100){
    hh = hh - 24;
  }else{
    hh = hh - curr_hour_window;
  }
  mm = mm/MIN_PER_HOUR;
  new_time = hh + mm;
  return std::make_tuple(new_time, curr_hour_window);
}

std::tuple<std::string, std::string>
determine_hour_windows(double start_time, double end_time){
  std::string str_hour_window1 = "";
  std::string str_hour_window2 = "";
  if( ((start_time <= 300)||(start_time > 2100)) && 
        ((end_time <= 300)||(end_time > 2100)) ){
      str_hour_window1 = "00";
  }
  if( ((start_time > 300)&&(start_time <= 900)) || 
              ((end_time > 300)&&(end_time <= 900)) ){
     if(!str_hour_window1.compare("")){
       str_hour_window1 = "06";
     }else{
       str_hour_window2 = "06";
     }
  }
  if( ((start_time > 900)&&(start_time <= 1500)) || 
              ((end_time > 900)&&(end_time <= 1500)) ){
     if(!str_hour_window1.compare("")){
       str_hour_window1 = "12";
     }else{
       str_hour_window2 = "12";
     }
  }
  if( ((start_time > 1500)&&(start_time <= 2100)) || 
              ((end_time > 1500)&&(end_time <= 2100)) ){
     if(!str_hour_window1.compare("")){
       str_hour_window1 = "18";
     }else{
       str_hour_window2 = "18";
     }
  }

  //THIS IS PART OF A NEW DAY
  if( ((start_time > 1500)&&(start_time <= 2100)) &&
		(str_hour_window1.compare("") != 0 )  ){
    str_hour_window2 = "00";
  }

  if( (!str_hour_window1.compare("")) && (!str_hour_window2.compare("")) ){
    throw OutOfBounds();
  }
  return std::make_tuple(str_hour_window1, str_hour_window2);
}

std::tuple<double, double>
find_time_range(const std::vector<std::string>& hms){
  double start_time, end_time, num_time;
  start_time = MAX_VALUE;
  end_time = MIN_VALUE;  
  for(auto& str_time : hms){
    num_time = stod(str_time);
    if(num_time < start_time){
      start_time = num_time;
    }
    if(num_time > end_time){
      end_time = num_time;
    }
  }
  return std::make_tuple(start_time, end_time);
}

std::tuple<std::vector<std::string>, std::vector<std::string>>
convert_to_uv(std::vector<std::string>& spd, std::vector<std::string>& dir){

  std::vector<std::string> uwnd, vwnd;
  double rad, math_dir;  
  for (auto tuple : boost::combine(spd, dir)){
    std::string str_spd, str_dir;
    boost::tie(str_spd, str_dir) = tuple;
    double num_spd, num_dir;
    num_spd = stod(str_spd);
    num_dir = stod(str_dir);

    //convert to math direction
    math_dir = 270 - num_dir;
    if(math_dir < 0){
      math_dir = math_dir + 360;
    }
    rad = (PI/180) * math_dir;
    uwnd.push_back( std::to_string((num_spd)*(cos(rad))) );
    vwnd.push_back( std::to_string((num_spd)*(sin(rad))) );
  }
  return std::make_tuple(uwnd, vwnd);
}

