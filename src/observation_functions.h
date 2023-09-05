//observation_functions.h

#ifndef OBSERVATION_FUNCTIONS_H_
#define OBSERVATION_FUNCTIONS_H_

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "observation_exceptions.h"

using json = nlohmann::json;

std::string set_subtype(nlohmann::json & sat);
std::string set_obstype(nlohmann::json & sat, std::string& type);

void skip_line(std::ifstream& infile);
std::string& remove_quotes(std::string& str);
std::string& retrieve_name(std::string& str);
std::string get_filename(const std::string& str);

std::string incrementFileDate(const std::string& oldDate);
std::string update_date(const std::string& oldDate,
  const std::string& time, const std::string& hour_window);

#endif //OBSERVATION_FUNCTIONS_H_
