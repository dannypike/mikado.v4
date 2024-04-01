#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_GLOBALS_H)
#define CMN_GLOBALS_H

#include "errorCodes.h"

// Websocket protocol version
#define MKO_WEBSOCKET_PROTOCOL_VERSION "1"

namespace mikado::common {
   extern std::filesystem::path MikadoExeFolder;
   extern bool MikadoShutdownRequested;

   typedef std::shared_ptr<ix::WebSocket> WebSocketPtr;

   // AppIds are used to identify the sending application in websocket messages
   extern std::string appIdBroker;
   extern std::string appIdGlobber;
   extern std::string appIdStorage;

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

   // Strings used in the configuration files
   extern std::string const cfgAppId;
   extern std::string const cfgArgs;
   extern std::string const cfgStartFolder;
   extern std::string const cfgExePath;

   // fields are used in websocket messages
   extern std::string fieldAction;
   extern std::string fieldAppId;
   extern std::string fieldTimestamp;
   extern std::string fieldType;
   extern std::string fieldVersion;

   // Actions are used to identify the type of message in websocket messages
   extern std::string actionConnect;

   MikadoErrorCode commonInitialize(int argc, char *argv[], std::function<void()> outputBanner);
   void commonShutdown();
} // namespace mikado::common

#endif // CMN_GLOBALS_H
