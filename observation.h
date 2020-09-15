//observation.h

#ifndef OBSERVATION_H_
#define OBSERVATION_H_

#include "observation_implementation/observation_implementation.h"

namespace obs{
  
  //debugging option for logging 
  const bool debug_logfile = true;

  class Observation{
   public:
     std::vector<std::string> evaluate_file(const std::string& filename, 
                              FileSource source, OutputFormat format);
   private:
     static std::unique_ptr<ObservationImpl> 
		obs_selection(FileSource source);
  };
}
     
#endif //OBSERVATION_H_
