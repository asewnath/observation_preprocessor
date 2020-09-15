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
                           FileSource source, OutputFormat format){
  std::unique_ptr<ObservationImpl> obsptr;
  obsptr = obs_selection(source);
  obsptr->set_file(filename);
  obsptr->preprocess();
  obsptr->output_data(format);
  //obsptr->output_filenames.push_back("dummy_file.bfr");
  return obsptr->output_filenames;
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
  throw SourceUnavailable();
}
