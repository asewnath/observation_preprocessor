//observation_exceptions.h

#ifndef OBSERVATION_EXCEPTIONS_H_
#define OBSERVATION_EXCEPTIONS_H_

//put this in different file
struct MissingObsType : public std::exception{
  const char * what () const throw(){
    return "Missing Observation Type";
  }
};

struct MissingSubType : public std::exception{
  const char * what () const throw(){
    return "Missing Satellite ID";
  }
};

struct DataNotFound : public std::exception{
  const char * what () const throw(){
    return "Data not found";
  }
};

struct OutOfBounds : public std::exception{
  const char * what () const throw(){
    return "Value out of bounds";
  }
};

struct ZeroCount : public std::exception{
  const char * what () const throw(){
    return "Observation count is zero";
  }
};

struct SourceUnavailable: public std::exception{
  const char * what () const throw() {
    return "Source is not available";
  }
};

struct OutputFormatUnavailable: public std::exception{
  const char * what () const throw() {
    return "Output Format is not available";
  }
};


struct TypeUnavailable: public std::exception{
  const char * what () const throw(){
    return "Type is not available";
  }
};

#endif //OBSERVATION_EXCEPTIONS_H_
