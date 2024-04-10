#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_ALGORITHMS_H)
#define CMN_ALGORITHMS_H

#include "common/globals.h"
#include "common/errorCodes.h"

namespace mikado::common {
   extern std::string EmptyString;

   std::string toLower(boost::json::string const &str);
   std::string toLower(std::string const &str);

   std::string trim(boost::json::string const &str);
   std::string trim(std::string_view str, std::string_view unwanted = common::kTrimDefaults);

   std::string toString(std::wstring const &str);
   std::wstring toString(std::string const &str);
   
   char const *toString(std::string &scratch, char const *format, ...);
   char const *toString(std::string &scratch, boost::posix_time::ptime const &time);

   std::string formatTime(std::string const &format, boost::posix_time::ptime const &time);
   std::string formatTime(std::string const &format, struct tm const *timeToUse = nullptr);
   std::string formatNow(char const *format = nullptr);
   std::string formatTimestamp();

   std::filesystem::path lexicalPath(std::filesystem::path const &filename, bool ensureAbsolute = true
      , std::filesystem::path const *fromFolder = nullptr);

   boost::json::value jsonProperty(boost::json::value const &value, std::string const &propertyName);
   std::string jsonString(boost::json::value const &value);
   boost::json::value jsonProperty(boost::json::value const &value, size_t index);
   std::string jsonPropertyString(boost::json::value const &value, std::string const &propertyName
      , char const *defaultValue = nullptr);
   MikadoErrorCode jsonVectorString(std::vector<std::string> &result, boost::json::value jv
      , char const *propertyName = nullptr);
   
   std::string poGetString(boost::program_options::variables_map const &cfg, std::string const &propertyName
      , char const *defaultValue = nullptr, std::source_location const &loc = std::source_location::current());
   std::vector<std::string> poGetVectorString(boost::program_options::variables_map const &cfg
      , std::string const &propertyName, bool required = false
      , std::source_location const &loc = std::source_location::current());
   
   template<class T>
   T poGet(boost::program_options::variables_map const &cfg, std::string const &propertyName
      , T const *defaultValue, std::source_location const &loc = std::source_location::current()) {

      try {
         return cfg.at(propertyName).as<T>();
      }
      catch (const std::exception &e) {
         // Return the default value
      }
      return defaultValue ? *defaultValue : T{};
   }

} // namespace mikado::common

#endif // CMN_ALGORITHMS_H
