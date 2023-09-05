//observation.cpp
//#include <boost/range/combine.hpp>
//#include <math.h>
//#include <iterator>
//#include <algorithm> //?
//#include <netcdf>
//#include <iomanip>
//#include <map>

#include "observation.h"

using namespace obs;

/* ----------------------------------------------------------------------------
Observation Member Definitions
---------------------------------------------------------------------------- */

std::vector<std::string>
Observation::evaluate_file(const std::string& filename, 
                           FileSource source, OutputFormat format,
                           const std::string& output_dir){
  try{
    std::unique_ptr<ObservationImpl> obsptr;
    obsptr = obs_selection(source);
    obsptr->set_file(filename);
    obsptr->preprocess();
    obsptr->output_data(format, output_dir);
    return obsptr->output_filenames;
  }catch(const std::exception& e){
    std::cout << "Unable to process file." << std::endl;
    std::vector<std::string> empty_exception;
    return empty_exception;
  }
}

std::unique_ptr<ObservationImpl> 
Observation::obs_selection(FileSource source){
  if(source == FileSource::fromNRL)
    return std::make_unique<NRLObservation>(NRLObservation());
  if(source == FileSource::fromWisconsin)
    return std::make_unique<UWObservation>(UWObservation());
  if(source == FileSource::fromGSI)
    return std::make_unique<GSIObservation>(GSIObservation());
  if(source == FileSource::fromEUMETSAT)
    return std::make_unique<METSATObservation>(METSATObservation());
  if(source == FileSource::fromJMA)
    return std::make_unique<JMAObservation>(JMAObservation());
  if(source == FileSource::fromNOAA)
    return std::make_unique<NOAAObservation>(NOAAObservation());
  if(source == FileSource::fromKMA)
    return std::make_unique<KMAObservation>(KMAObservation());
  if(source == FileSource::fromAVHRR)
    return std::make_unique<AVHRRObservation>(AVHRRObservation());
  throw SourceUnavailable();
}
