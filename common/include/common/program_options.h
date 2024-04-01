#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_OPTIONS_H)
#define CMN_OPTIONS_H

namespace mikado::common {

   // Strings used in boost::program_options descriptions
   extern std::string const poAppStart;
   extern std::string const poBrokerHost;
   extern std::string const poBrokerPort;
   extern std::string const poBrokerTimeout;
   extern std::string const poConsoleRestoreOnExit;
   extern std::string const poConsoleQuiet;
   extern std::string const poConsoleTitle;
   extern std::string const poDefault;
   extern std::string const poExclude;
   extern std::string const poHelp;
   extern std::string const poInclude;
   extern std::string const poRoot;

} // namespace mikado::common

#endif // CMN_OPTIONS_H
