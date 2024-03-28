// Copyright (c) 2024 Gamaliel Ltd

#include "common/algorithms.h"
#include "common/globals.h"
#include "common/logger.h"

namespace bt = boost::posix_time;
namespace json = boost::json;
namespace po = boost::program_options;
using namespace std;
using namespace std::filesystem;

namespace mikado::common {

   ///////////////////////////////////////////////////////////////////////
   //
   string toLower(json::string const &str) {
      string result;
      transform(str.begin(), str.end(), back_inserter(result), ::tolower);
      return result;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string toLower(string const &str) {
      string result;
      transform(str.begin(), str.end(), back_inserter(result), ::tolower);
      return result;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string trim(json::string const &jvString) {
      string str(jvString);
      string::size_type start = str.find_first_not_of(" \t");
      if (start != json::string::npos) {
         string::size_type end = str.find_last_not_of(" \t");
         str = str.substr(start, end - start + 1);
      }
      return str;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string trim(string const &str) {
      string result;
      string::size_type start = str.find_first_not_of(" \t");
      if (start != string::npos) {
         string::size_type end = str.find_last_not_of(" \t");
         result = str.substr(start, end - start + 1);
      }
      return result;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string toString(wstring const &str) {
      return boost::locale::conv::utf_to_utf<char>(str);
   }

   ///////////////////////////////////////////////////////////////////////
   //
   wstring toString(string const &str) {
      return boost::locale::conv::utf_to_utf<wchar_t>(str);
   }

   ///////////////////////////////////////////////////////////////////////
   //
   char const *toString(string &scratch, char const *format, ...)
   {
      va_list ap;
      va_start(ap, format);

      while (true) {
         auto capacity = scratch.capacity();
         auto written = vsnprintf(scratch.data(), capacity - 1, format, ap);
         if (0 > written) {
            str_error() << "Error formatting string: " << format;
            break;
         }
         if (written < capacity) {
            break;
         }
         scratch.resize(written + 1);
      }
      va_end(ap);
      return scratch.c_str();
   }

   ///////////////////////////////////////////////////////////////////////
   //
   char const *toString(string &scratch, bt::ptime const &time) {
      scratch = move(to_iso_extended_string(time));
      return scratch.c_str();
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string formatTime(string const &format, bt::ptime const &time) {
      auto tmStruct = boost::posix_time::to_tm(time);
      return formatTime(format, &tmStruct);
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string formatTime(string const &format, struct tm const *timeToUse) {
      struct tm tmStruct;
      if (!timeToUse) {
         time_t now = time(nullptr);
         if (0 != gmtime_s(&tmStruct, &now)) {
            return string("<gmtime_s failed>");
         }
         timeToUse = &tmStruct;
      }

      auto length = format.size() + 80;
      char *buffer = (char *)_malloca(length * sizeof(*buffer));
      length = buffer ? strftime(buffer, length, format.c_str(), timeToUse) : 0;
      return move(string(buffer, length));
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string formatNow(char const *format) {
      struct tm tmStruct;
      time_t now = time(nullptr);
      if (0 != gmtime_s(&tmStruct, &now)) {
         return string("<gmtime_s failed>");
      }
      format = format ? format : "%H:%M:%S";
      return formatTime(format, &tmStruct);
   }

   ///////////////////////////////////////////////////////////////////////
   //
   path lexicalPath(path const &filename, bool ensureAbsolute
      , path const *fromFolder) {
      path result;
      if (filename.is_relative()) {
         if (fromFolder) {
            if (ensureAbsolute && fromFolder->is_relative()) {
               result = MikadoExeFolder / *fromFolder / filename;
            }
            else {
               result = *fromFolder / filename;
            }
         }
         else {
            result = MikadoExeFolder / filename;
         }
      }
      else if (ensureAbsolute && filename.is_relative()) {
         result = MikadoExeFolder / filename;
      }
      else {
         result = filename;
      }
      return move(result.lexically_normal());
   }

   ///////////////////////////////////////////////////////////////////////
   //
   json::value jsonProperty(json::value const &value, string const &propertyName) {
      if (value.is_object()) {
         auto jo = value.as_object();
         if (jo.contains(propertyName.c_str())) {
            return value.at(propertyName);
         }

         string keys;
         for (auto kvp : jo) {
            if (keys.empty()) {
               keys = "expected to see a value called '" + propertyName
                  + "', but only see values called: '";
            }
            else {
               keys += "', '";
            }
            keys += kvp.key_c_str();
         }
         if (!keys.empty()) {
            str_warn() << keys << "'" << endl;
         }
         else {
            str_warn() << "json value is a leaf node - looking for a sub-key called '" << propertyName
               << "', but none found" << endl;
         }
      }
      return json::value();
   }

   ///////////////////////////////////////////////////////////////////////
   //
   json::value jsonProperty(json::value const &value, size_t index) {
      if (value.is_array()) {
      json::array const &arr = value.as_array();
         if (index < arr.size()) {
            return arr.at(index);
         }
      }
      return json::value();
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string jsonString(json::value const &value) {
      if (value.is_string()) {
         return value.as_string().c_str();
      }
      return string();
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string jsonPropertyString(json::value const &jv, string const &propertyName
      , char const *defaultValue)
   {
      if (jv.is_object()) {
         if ((jv.as_object()).contains(propertyName.c_str())) {
            return jsonString(jv.at(propertyName));
         }
         else if (!defaultValue) {
            // If there is no default value, then we will log an error
            str_error() << "json value does not have property '" << propertyName << "'" << endl;
         }
      }
      else if (!defaultValue) {
         // If there is no default value, then we will log an error
         str_error() << "json value is not an object - cannot find property '" << propertyName << "'" << endl;
      }
      return defaultValue ? defaultValue : string();
   }

   ///////////////////////////////////////////////////////////////////////
   //
   string poGetString(po::variables_map const &cfg, string const &propertyName
      , char const *defaultValue, source_location const &loc) {

      try {
         return cfg.at(propertyName).as<string>();
      }
      catch (const std::exception &e) {
         // If there is no default value, then we will log an error
         if (!defaultValue) {
            log_exception(e);
            MikadoLog::MikadoLogger.streamError(loc)
               << "configuration does not have a property called '" << propertyName << "'" << endl;
         }
      }
      return move(defaultValue ? string(defaultValue) : string());
   }

   ///////////////////////////////////////////////////////////////////////
   //
   vector<string> poGetVectorString(po::variables_map const &cfg, string const &propertyName
      , bool required, source_location const &loc) {

      try {
         if (cfg.count(propertyName)) {
            return cfg.at(propertyName).as<vector<string>>();
         }
      }
      catch (const std::exception &e) {
         // If there is no default value, then we will log an error
         if (required) {
            log_exception(e);
            MikadoLog::MikadoLogger.streamError(loc)
               << "configuration does not have a property called '" << propertyName << "'" << endl;
         }
      }
      return vector<string>();
   }

} // namespace mikado::common
