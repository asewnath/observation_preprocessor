//observation_implementation.h

#ifndef OBSERVATION_IMPLEMENTATION_H_
#define OBSERVATION_IMPLEMENTATION_H_

#include <map>
#include <math.h>
#include <netcdf>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iostream>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <boost/range/combine.hpp>

#include "input_output.h"
#include "observation_functions.h"
#include "observation_exceptions.h"

using json = nlohmann::json;

namespace obs{

  const int WS_WIDTH = 12;
  const double MIN_VALUE = 0;
  const double MISS = 9999.9;
  const double MAX_VALUE = 9999;
  const double MIN_PER_HOUR = 60;
  const std::string MISS_STR = "9999.9";
  const double PI = 3.14159265358979323846264;
  const std::string JSON_SATLIST = 
        "./observation_implementation/satellite_id.json";
   
  const double MONTHS_IN_YEAR = 12;
  const double MAX_DAYS_IN_MONTH = 31;
  const std::array<double, 4> THIRTY_DAY_MONTHS = {4, 6, 9, 11};
  const double THIRTY_DAYS = 30;
  const double FEBRUARY = 2;
  const double MAX_DAYS_IN_FEB = 29;

 
  class ObservationImpl{

   public:
     ObservationImpl();
     virtual ~ObservationImpl();
     virtual void preprocess() = 0;
     void set_file(const std::string& filename);
     void output_data(OutputFormat format);
     std::vector<std::string> output_filenames;    
   protected: 
     std::string filename, obs_format, stationid;
     std::string subset = "SATWND";
     //std::string tablefn = 
     //   "./observation_implementation/write_prep/qscat_tabl";      
     std::string tablefn = "prepobs_prep.bufrtable";
     long ndate = 0;
     int obs_count = 0;
     std::vector<std::pair<std::string,std::vector<std::string>>> data_storage;

    private:
     void convert_to_prepbufr();
     void convert_to_textfile();
     void convert_to_hdf();
     std::vector<double> find_data(std::vector<std::string> &str_vect);
  };

  class NRLObservation: public ObservationImpl{
   
   public:
     NRLObservation();
     ~NRLObservation();
     void preprocess();        
   private:
     void import_file();
     void load_data();
     void populate_metadata(const std::string& str);
  };

  class UWObservation: public ObservationImpl{

   public:
     UWObservation();
     ~UWObservation();
     void preprocess();
   private:

     void set_date(const std::string& hour_window1,
                    const std::string& hour_window2);
     void time_conversion();
     void uv_conversion();
     void find_numerical_id();
     void import_file();
     void load_data();
     void do_conversions();
     void lon_conversion();
     void setting_qi();
  };
 

  class GSIObservation: public ObservationImpl{
  
   public:
    GSIObservation();
    ~GSIObservation();
    void preprocess();
   
   private:
    void import_file();
    void load_data();
  };

  class METSATObservation: public ObservationImpl{
  
   public:
    METSATObservation();
    ~METSATObservation();
    void preprocess();
  
   private:
     void import_file();

  };

  class JMAObservation: public ObservationImpl{
  
   public:
    JMAObservation();
    ~JMAObservation();
    void preprocess();

  };
}



std::tuple<double, double> calculate_time(const std::string& old_time);
std::tuple<std::string, std::string> 
            determine_hour_windows(double start_time, double end_time);
std::tuple<double, double>
            find_time_range(const std::vector<std::string>& hms);
std::tuple<std::vector<std::string>, std::vector<std::string>>
convert_to_uv(std::vector<std::string>& spd, std::vector<std::string>& dir);

std::map<std::string, std::vector<std::string>>
construct_obs_labels_map();

std::string
UpdateFileDate(const std::string& oldDate);



#endif //OBSERVATION_IMPLEMENTATION_H_

