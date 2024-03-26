#pragma once
#if !defined(CMN_ALGORITHMS_H)
#define CMN_ALGORITHMS_H

namespace mikado::common {
   extern std::string EmptyString;

   std::string toLower(boost::json::string const &str);
   std::string toLower(std::string const &str);

   std::string trim(boost::json::string const &str);
   std::string trim(std::string const &str);

   std::string toString(std::wstring const &str);
   std::wstring toString(std::string const &str);
   
   char const *toString(std::string &scratch, char const *format, ...);
   char const *toString(std::string &scratch, boost::posix_time::ptime const &time);

   std::string formatTime(std::string const &format, boost::posix_time::ptime const &time);
   std::string formatTime(std::string const &format, struct tm const *timeToUse = nullptr);
   std::string formatNow(char const *format = nullptr);

   std::filesystem::path lexicalPath(std::filesystem::path const &filename, bool ensureAbsolute = true
      , std::filesystem::path const *fromFolder = nullptr);

   boost::json::value jsonProperty(boost::json::value const &value, std::string const &propertyName);
   std::string jsonString(boost::json::value const &value);
   boost::json::value jsonProperty(boost::json::value const &value, size_t index);
   std::string jsonPropertyString(boost::json::value const &value, std::string const &propertyName
      , char const *defaultValue = nullptr);

   std::string poGetString(boost::program_options::variables_map const &cfg, std::string const &propertyName
      , char const *defaultValue = nullptr, std::source_location const &loc = std::source_location::current());
   std::vector<std::string> poGetVectorString(boost::program_options::variables_map const &cfg
      , std::string const &propertyName, bool required = false
      , std::source_location const &loc = std::source_location::current());
   bool poGetBool(boost::program_options::variables_map const &cfg, std::string const &propertyName
      , bool const *defaultValue = nullptr, std::source_location const &loc = std::source_location::current());
   unsigned poGetUnsigned(boost::program_options::variables_map const &cfg, std::string const &propertyName
      , unsigned const *defaultValue = nullptr, std::source_location const &loc = std::source_location::current());
   float poGetFloat(boost::program_options::variables_map const &cfg, std::string const &propertyName
      , float const *defaultValue = nullptr, std::source_location const &loc = std::source_location::current());

} // namespace mikado::common

#endif // CMN_ALGORITHMS_H
