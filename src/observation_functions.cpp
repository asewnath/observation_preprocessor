//observation_functions.cpp

#include "observation_functions.h"
#include <iostream>

void
skip_line(std::ifstream& infile){
  std::string str;
  std::getline(infile,str);
}

std::string&
remove_quotes(std::string & str){
  str.erase(remove(str.begin(), str.end(), '\"'), str.end());
  return str;
}

std::string&
retrieve_name(std::string& str){
  remove_quotes(str);
  return str;
}

std::string
get_filename(const std::string& str){
  char delimiter = '/';
#ifdef _WIN32
  delimiter = '\\';
#endif

  //check if delimiter exists
  if( std::find(str.begin(), str.end(), delimiter) == str.end() ){
    return str;
  } 
    
  size_t pos = str.rfind(delimiter, str.length());
  if (pos != std::string::npos){
    return(str.substr(pos+1, str.length() - pos));
  }else{
    return "";
  }
}

std::string
update_date(const std::string& old_date, const std::string& time, const std::string& hour_window){

  double d_time;
  double d_hour_window;

  d_time = stod(time);
  d_hour_window = stod(hour_window);

  //if time is negative and hour is zero, increment the time window
  if((d_time <= -0.5) && (d_hour_window == 0)){
    return incrementFileDate(old_date);
  }

  return old_date;
}




std::string
incrementFileDate(const std::string& oldDate){
  /*
    This function takes in an old date and updates by
    one day if hour window is 0 and time is negative.
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

  const double MONTHS_IN_YEAR = 12;
  const double MAX_DAYS_IN_MONTH = 31;
  const std::array<double, 4> THIRTY_DAY_MONTHS = {4, 6, 9, 11};
  const double THIRTY_DAYS = 30;
  const double FEBRUARY = 2;
  const double MAX_DAYS_IN_FEB = 29;

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
  else if( (dMonth == FEBRUARY)&&(dDay >= 28)  ){
    std::cout << "in february handling" << std::endl;
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
    dMonth +=1;
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


