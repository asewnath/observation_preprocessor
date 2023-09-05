//observation.cpp

#include "observation_implementation.h"
#include <fstream>
#include <cstdint>
#include <experimental/filesystem>
#include <valarray>
#include "Eigen/Dense"
#include "unsupported/Eigen/CXX11/Tensor"


using namespace obs;
namespace fs = std::experimental::filesystem;

extern "C" {
  void obs_endbufr();
  void obs_initbufr(const char* outfn, int* len_out, 
                    const char* tablefn, int* len_tab);

  void obs_write_wind(const char * stationid, double* lon, double* lat,
           double* time, double* obs_type, double* pres, double* uwnd, 
           double* vwnd, double* pres_qcflag, double* wind_qcflag, 
           double* pres_error, double* wind_error, const char* subset,
           int* ndate, const double* elv,double* sat_id, double* rffl,
	   double* qify, double* qifn, double* eeqf, double* saza,
           double* eham );
}


/* ----------------------------------------------------------------------------
Observation Implementation Member Definitions
---------------------------------------------------------------------------- */

ObservationImpl::ObservationImpl(){};
ObservationImpl::~ObservationImpl(){};

std::string
UpdateFileDate(const std::string& oldDate){
  /*
    This function takes in an old date and updates by
    one day if the second hour window is 0.
    Parameters:
      oldDate - date in the format YYYYMMDDH1(H2).
    Returns: 
      newDate - date incremented by 1.

    Checks:
      -if oldDate is a garbage value 
  */


  //Extract year, month, and date and set them to numerical values
  double dYear  = stod(oldDate.substr(0,4));
  double dMonth = stod(oldDate.substr(4,2));
  double dDay   = stod(oldDate.substr(6,2));  
 


  //Checking the input
  if( (dMonth > MONTHS_IN_YEAR)||(dDay > MAX_DAYS_IN_MONTH)  ) throw OutOfBounds();
   
  if(std::any_of(THIRTY_DAY_MONTHS.begin(), THIRTY_DAY_MONTHS.end(), [&dMonth](double val){
    return val == dMonth;
  }) && (dDay > THIRTY_DAYS)  ) throw OutOfBounds();
  
  if( (dMonth == FEBRUARY)&&(dDay > MAX_DAYS_IN_FEB)  ) throw OutOfBounds();

  //Handle 30-day months
  if(std::any_of(THIRTY_DAY_MONTHS.begin(), THIRTY_DAY_MONTHS.end(),
     [&dMonth](double val){ return val == dMonth;  })
      && (dDay == THIRTY_DAYS)  ){
    dDay = 1;
    dMonth += 1;
  }
  //Handle February
  else if( (dMonth == FEBRUARY)||(dDay >= 28)  ){
    if(dDay == 29){
      dDay = 1;
      dMonth += 1;
    }else{
      //Determine if it's a leap year
      if( ((static_cast<int>(dYear)%4 == 0) && (static_cast<int>(dYear)%100 != 0)) || 
        ((static_cast<int>(dYear)%4 == 0) && (static_cast<int>(dYear)%100 == 0) && (int(dYear)%400 == 0)) ){  
        dDay += 1;
      }else{
        dDay = 1;
        dMonth += 1;
      }
    }
  }
  //Handle when day is equal to 31
  else if(dDay == 31){
    dDay = 1;   
  }else{dDay += 1;}

  //Handle month and year
  if( dMonth > MONTHS_IN_YEAR  ){
    dMonth = 1;
    dYear += 1;
  }

  //Convert date month and year back to strings with two digits.
   double numDigits = 2;
   std::string sDay  = std::to_string(static_cast<int>(dDay));
   std::string sMonth = std::to_string(static_cast<int>(dMonth));
   std::string sYear  = std::to_string(static_cast<int>(dYear));
 
   //change logic here to modify if length is not equal to two
 
   if(sMonth.length() != 2){
     sMonth = "0" + sMonth;
   } 
   if(sDay.length() != 2){
     sDay = "0" + sDay;
   }

   if(sYear.length() != 4) throw OutOfBounds();

  //Concatenate the strings
  std::string newDate = sYear+sMonth+sDay;

  //return the string
  return newDate;
}


void
ObservationImpl::set_file(const std::string& filename){
  this->filename = filename;
}

void
ObservationImpl::output_data(OutputFormat format, const std::string & output_dir){
  if(format == OutputFormat::toPrepbufr){
    convert_to_prepbufr(output_dir);
  }else if(format == OutputFormat::toTextFile){
    convert_to_textfile(output_dir);
  }else{
    throw OutputFormatUnavailable();
  }
}

std::vector<double>
ObservationImpl::find_data( std::vector<std::string> &str_vect ){
  int size = this->obs_count;
  std::vector<double> data_vect(size);
  for(auto const& x : this->data_storage){

    try{
      std::string key = x.first;
      if(std::any_of(str_vect.begin(), str_vect.end(), [&key](const std::string & str){
        return key.find(str) != std::string::npos;
      }) ){       
        //return a vector of values that have been transformed into doubled
        std::transform(x.second.begin(), x.second.end(), data_vect.begin(), 
                [](std::string const& val){return stod(val);}); 
             
      }
    }catch(const std::exception& e ){
      std::cout << e.what() << std::endl;
      std::cout << "Couldn't find " << str_vect[0] << std::endl;
      throw DataNotFound(); 
    }
    
  }
  
  //wqcflag and pqcflag default value
  if ( (str_vect[0].compare("wqcflag") == 0) || (str_vect[0].compare("pqcflag") == 0) ){
    if( std::all_of(data_vect.begin(), data_vect.end(), [](double i) { return i==0; }) ){
      data_vect.assign(this->obs_count, 2);
    }
  }
  
  //set pres_err, wind_err, rffl, eeqf, saza, eham
    if ( (str_vect[0].compare("pres_err") == 0) || (str_vect[0].compare("wind_err") == 0) ||
         (str_vect[0].compare("rffl") == 0) || (str_vect[0].compare("eeqf") == 0) ||
         (str_vect[0].compare("saza") == 0) || (str_vect[0].compare("eham") == 0)){
      if( std::all_of(data_vect.begin(), data_vect.end(), [](double i) { return i==0; }) ){
        data_vect.assign(this->obs_count, MISS);
      }
    } 
  //set qify, qifn
  if ( (str_vect[0].compare("qify") == 0) || (str_vect[0].compare("qifn") == 0) ){
    if( std::all_of(data_vect.begin(), data_vect.end(), [](double i) { return i==0; }) ){
      data_vect.assign(this->obs_count, MISS);
    }else{
      //Check if QI is in percentages and change it if it's not
      double max = *std::max_element(data_vect.begin(), data_vect.end());
      if(max <= 1){
        std::transform(data_vect.begin(), data_vect.end(), data_vect.begin(),
              std::bind(std::multiplies<double>(), std::placeholders::_1, 100));
      }
    }
  }
 
  return data_vect;
}

void
ObservationImpl::convert_to_prepbufr(const std::string & output_dir){
  //if ndate isn't the length you expect, that's because there's an extra hour window
  std::vector<std::string> hour_windows;
  std::string str_ndate = std::to_string(this->ndate);

  if(str_ndate.length() != 10){
    //two hour windows present
    hour_windows.push_back(str_ndate.substr(8, 2));
    hour_windows.push_back(str_ndate.substr(10, 2));
  }else{
    //one hour window present
    hour_windows.push_back(str_ndate.substr(8));
  }

  bool dateChange = false;
  int hwCount = 0;
  for(auto hour_window : hour_windows){
    
    double master_hour_window = stod(hour_window);
    std::string currHw = hour_window;
    std::string oldDate = str_ndate;
    std::string fileDate;

    hwCount++;
    fileDate = str_ndate.substr(0, 8); //remove the hour windows
  
    std::stringstream ss(this->filename);
    std::vector<std::string> fname_tokens;
    std::string token;
    while(std::getline(ss, token, '/')){
      fname_tokens.push_back(token);
    } 
       
    //std::string path_str = output_dir;
    fs:: path p1 = output_dir;
    if(!fs::exists(p1)){
      fs::create_directory(p1);
    }
    //make output directories if needed 
    fs::path p2 = p1.string()+"/"+this->stationid;
    if(!fs::exists(p2)){
      fs::create_directory(p2);
    }
    fs::path p3 = p2.string()+"/Y"+fileDate.substr(0,4);
    if(!fs::exists(p3)){
      fs::create_directory(p3);
    }
    fs::path p4 = p3.string()+"/M"+fileDate.substr(4,2);
    if(!fs::exists(p4)){
      fs::create_directory(p4);
    }
    fs::path p5 = p4.string()+"/prepbufr/";
    if(!fs::exists(p5)){
      fs::create_directory(p5);
    }

    std::string return_dir = p4.string();
    std::string output_directory = p5.string();
    std::string outfn = output_directory + fname_tokens.back() + "-" + fileDate + "-" +  hour_window + ".bufr";
    
    this->output_filenames.push_back(return_dir);
    int len_out = outfn.size();
    int len_tab = this->tablefn.size();
    int size = this->obs_count;
    
    //run once to create labels map
    std::map<std::string, std::vector<std::string>> obs_labels = construct_obs_labels_map();
    std::vector<std::string> label_vect;  

    auto position = obs_labels.find("lat");
    label_vect = position->second;
    std::vector<double> lat = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("lon");
    label_vect = position->second;
    std::vector<double> lon = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("time");
    label_vect = position->second;
    std::vector<double> time = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("pres");
    label_vect = position->second;
    std::vector<double> pres = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("uwnd");
    label_vect = position->second;
    std::vector<double> uwnd = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("vwnd");
    label_vect = position->second;
    std::vector<double> vwnd = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("obstype");
    label_vect = position->second;
    std::vector<double> obstype = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("satid");
    label_vect = position->second;
    std::vector<double> satid = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("pqcflag");
    label_vect = position->second;
    std::vector<double> pqcflag = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("wqcflag");
    label_vect = position->second;
    std::vector<double> wqcflag = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("pres_err");
    label_vect = position->second;
    std::vector<double> pres_err = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("wind_err");
    label_vect = position->second;
    std::vector<double> wind_err = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("rffl");
    label_vect = position->second;
    std::vector<double> rffl = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("qify");
    label_vect = position->second;
    std::vector<double> qify = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("qifn");
    label_vect = position->second;
    std::vector<double> qifn = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("eeqf");
    label_vect = position->second;
    std::vector<double> eeqf = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("hour_window");
    label_vect = position->second;
    std::vector<double> f_hour_window  = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("saza");
    label_vect = position->second;
    std::vector<double> saza  = find_data(label_vect);
    label_vect.clear();

    position = obs_labels.find("eham");
    label_vect = position->second;
    std::vector<double> eham  = find_data(label_vect);
    label_vect.clear();

    std::vector<double>::iterator it_pres_err = pres_err.begin();
    std::vector<double>::iterator it_wind_err = wind_err.begin();

    std::vector<double>::iterator it_rffl = rffl.begin();
    std::vector<double>::iterator it_qify = qify.begin();
    std::vector<double>::iterator it_qifn = qifn.begin();
    std::vector<double>::iterator it_eeqf = eeqf.begin();
    std::vector<double>::iterator it_saza = saza.begin();
    std::vector<double>::iterator it_eham = eham.begin();

    //std::vector<std::string>::iterator it_hour_window = hour_window_vect.begin(); 
    std::vector<double>::iterator it_hour_window = f_hour_window.begin();    

    obs_initbufr( outfn.c_str(), &len_out, this->tablefn.c_str(), &len_tab);
  
    //create ndate  
    int final_ndate = stoi(std::to_string(this->ndate).substr(0, 8) + hour_window);
    //Boost combine seems to not like having more than ten arguments  
    for( auto tuple: boost::combine(lon, lat, time, pres, uwnd, vwnd,
                          satid, obstype, pqcflag, wqcflag) ) {
          
      double lon, lat, time, pres, uwnd, vwnd, satid, obstype, pqcflag, 
             wqcflag, pres_err, wind_err, rffl, qify, qifn, eeqf, saza,
             eham;
      boost::tie(lon, lat, time, pres, uwnd, vwnd, 
                 satid, obstype, pqcflag, wqcflag) = tuple;

      pres_err = *it_pres_err;
      wind_err = *it_wind_err;      
      rffl = *it_rffl;
      qify = *it_qify;
      qifn = *it_qifn;
      eeqf = *it_eeqf;
      saza = *it_saza;
      eham = *it_eham;

      double curr_hour_window = *it_hour_window;      

      if(master_hour_window == curr_hour_window){
        obs_write_wind( this->stationid.c_str(), &lon, &lat, &time, &obstype, 
    		    &pres, &uwnd, &vwnd, &pqcflag, &wqcflag, &pres_err, 
                    &wind_err, this->subset.c_str(), &final_ndate, &MISS, 
                    &satid, &rffl, &qify, &qifn, &eeqf, &saza, &eham);
        
      }
      ++it_pres_err;
      ++it_wind_err;
      ++it_hour_window;
     
      ++it_rffl;
      ++it_qify;
      ++it_qifn;
      ++it_eeqf;
      ++it_saza;
      ++it_eham;

    }
    obs_endbufr(); 
  }
}



void
ObservationImpl::convert_to_textfile(const std::string & output_dir){
  
  //check if this->data is empty
  if(this->data_storage.empty()){
     throw DataNotFound();
  }
  
  //if ndate isn't the length you expect, that's because there's an extra hour window
  std::vector<std::string> hour_windows;
  std::string fileDate;
  std::string str_ndate = std::to_string(this->ndate); 

  if(str_ndate.length() != 10){
    //two hour windows present
    hour_windows.push_back(str_ndate.substr(8, 2));
    hour_windows.push_back(str_ndate.substr(10, 2));
  }else{
    //one hour window present
    hour_windows.push_back(str_ndate.substr(8));
  }
  
  std::vector<std::string> hour_window_vect;
  for(auto const& x : this->data_storage){
    if( (x.first.find("hour_window") != std::string::npos) ){
      hour_window_vect = x.second; 
    }
  }
  if(hour_window_vect.empty())
    throw DataNotFound();
  
  for(auto curr_hour_window : hour_windows){

    //break down eumetsat filename to retrieve datetime  
    std::stringstream ss(this->filename);
    std::vector<std::string> fname_tokens;
    std::string token;
    while(std::getline(ss, token, '/')){
      fname_tokens.push_back(token);
    }

    fileDate = str_ndate.substr(0, 8); //remove the hour windows

    //create directories if they don't exist
    fs::path p1 = output_dir;
    if(!fs::exists(p1)){
      fs::create_directory(p1);
    }
    fs::path p2 = p1.string()+"/"+this->stationid;
    if(!fs::exists(p2)){
      fs::create_directory(p2);
    }
    fs::path p3 = p2.string()+"/Y"+fileDate.substr(0,4);
    if(!fs::exists(p3)){
      fs::create_directory(p3);
    }
    fs::path p4 = p3.string()+"/M"+fileDate.substr(4,2);
    if(!fs::exists(p4)){
      fs::create_directory(p4);
    }
    fs::path p5 = p4.string()+"/text/";
    if(!fs::exists(p5)){
      fs::create_directory(p5);
    }

    std::string return_dir = p4.string();
    std::string output_directory = p5.string();
    std::string outfn = output_directory + fname_tokens.back() + "-" + curr_hour_window + "_processed.txt";
   
    this->output_filenames.push_back(return_dir);
    std::fstream file;
    file.open(outfn, std::fstream::out);
    file << std::left;

    //print the labels
    for(auto const& x : this->data_storage){
        file << std::setw(WS_WIDTH) << x.first;
    }  
    file << std::endl;

    //print the data
    for(int index=0; index < this->obs_count; ++index){

      if(hour_window_vect.at(index) == curr_hour_window){
      
        for(auto const& x : this->data_storage){
          file << std::setw(WS_WIDTH) << x.second.at(index); 
        }
        file << std::endl;

      }
    }
    file.close();
  }
  
}

std::map<std::string, std::vector<std::string>>
construct_obs_labels_map(){
  //initializing
  std::map<std::string, std::vector<std::string>> obs_labels;

  //lat
  std::vector<std::string> lat_labels = {"lat", "LAT", "Latitude"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("lat", lat_labels) );

  //lon
  std::vector<std::string> lon_labels = {"lon", "LON", "Longitude"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("lon", lon_labels) );
  
  //time
  std::vector<std::string> time_labels = {"time", "TIME"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("time", time_labels) );
  
  //pres
  std::vector<std::string> pres_labels = {"pres", "pre", "PRES", "Pressure", "height"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("pres", pres_labels) );
   
  //uwnd
  std::vector<std::string> uwnd_labels = {"uwnd", "u (m/s)", "u_Observation"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("uwnd", uwnd_labels) );

  //vwnd
  std::vector<std::string> vwnd_labels = {"vwnd", "v (m/s)", "v_Observation"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("vwnd", vwnd_labels) );

  //obstype
  std::vector<std::string> obstype_labels = {"obstype", "obs_type", "Observation_Type"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("obstype", obstype_labels) );

  //satid
  std::vector<std::string> satid_labels = {"satid", "sat_id", "Observation_Subtype"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("satid", satid_labels) );

  //pqcflag
  std::vector<std::string> pqcflag_labels = {"pqcflag"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("pqcflag", pqcflag_labels) );

  //wqcflag
  std::vector<std::string> wqcflag_labels = {"wqcflag"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("wqcflag", wqcflag_labels) );

  //pres_err
  std::vector<std::string> pres_err_labels = {"pres_err"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("pres_err", pres_err_labels) );

  //wind_err
  std::vector<std::string> wind_err_labels = {"wind_err"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("wind_err", wind_err_labels) );

  //rffl
  std::vector<std::string> rffl_labels = {"rffl"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("rffl", rffl_labels) );

  //qify
  std::vector<std::string> qify_labels = {"qify"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("qify", qify_labels) );

  //qifn
  std::vector<std::string> qifn_labels = {"qifn"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("qifn", qifn_labels) );

  //saza
  std::vector<std::string> saza_labels = {"saza"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("saza", saza_labels) );

  //eham
  std::vector<std::string> eham_labels = {"eham"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("eham", eham_labels) );

  //eeqf
  std::vector<std::string> eeqf_labels = {"eeqf"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("eeqf", eeqf_labels) );

 //hour_window
  std::vector<std::string> hour_window_labels = {"hour_window"};
  obs_labels.insert(std::pair<std::string, std::vector<std::string>>("hour_window", hour_window_labels) );

  return obs_labels;
}
