// Copyright (c) 2024 Gamaliel Ltd
#include "common.h"

namespace po = boost::program_options;
using namespace std;
using namespace std::filesystem;

namespace mikado::common {

   ///////////////////////////////////////////////////////////////////////////
   //
   po::options_description_easy_init Configure::addOptions() {
      return options_.add_options();
   }
   
   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::importFile(path const &cfgFilename) {
      if (!exists(cfgFilename.string())) {
         return MikadoErrorCode::MKO_ERROR_PATH_NOT_FOUND;
      }

      auto parsedFile = po::parse_config_file<char>(cfgFilename.string().c_str(), options_);
      po::store(parsedFile, values_, true);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::importCommandline(int argc, char *argv[]) {
      auto parsedCmdline = po::parse_command_line(argc, argv, options_);
      po::store(parsedCmdline, values_, true);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::notify() {
      po::notify(values_);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

} // namespace mikado::common
