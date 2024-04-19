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
   extern std::string appIdTorchBox;
   extern std::string appIdStorage;

   // String constants used in configuration files and command-line arguments
   extern std::string const kAppId;
   extern std::string const kArgs;
   extern std::string const kBrokerHost;
   extern std::string const kBrokerPort;
   extern std::string const kBrokerTimeout;
   extern std::string const kConsoleQuiet;
   extern std::string const kConsoleRestoreOnExit;
   extern std::string const kConsoleTitle;
   extern std::string const kCudaDevice;
   extern std::string const kDefault;
   extern std::string const kDevice;
   extern std::string const kExclude;
   extern std::string const kExePath;
   extern std::string const kHelp;
   extern std::string const kInclude;
   extern std::string const kInstanceId;
   extern std::string const kMakeMore;
   extern std::string const kMakeMoreContextSize;
   extern std::string const kMakeMoreEmbeddingDim;
   extern std::string const kMakeMoreHiddenLayer;
   extern std::string const kMakeMoreMaxNames;
   extern std::string const kMakeMoreNamesFile;
   extern std::string const kMakeMoreTrainingBatch;
   extern std::string const kMakeMoreTrainingBatchUpdateRate;
   extern std::string const kMakeMoreTrainingSteps;
   extern std::string const kModel;
   extern std::string const kMulMat;
   extern std::string const kRoot;
   extern std::string const kSixSpaces;
   extern std::string const kStartApp;
   extern std::string const kStartComspec;
   extern std::string const kStartFolder;
   extern std::string const kStartRetry;
   extern std::string const kTensorC;
   extern std::string const kTensorW1;
   extern std::string const kTensorW2;
   extern std::string const kTensorB2;
   extern std::string const kTensorBNGain;
   extern std::string const kTensorBNBias;
   extern std::string const kWindowBroker;

   // Miscellaneous string constants
   extern std::string const kTrimDefaults;

   // fields are used in websocket messages
   extern std::string fieldAction;
   extern std::string fieldAppId;
   extern std::string fieldInstanceId;
   extern std::string fieldTimestamp;
   extern std::string fieldType;
   extern std::string fieldVersion;

   // Actions are used to identify the type of message in websocket messages
   extern std::string actionConnect;

   MikadoErrorCode commonInitialize(int argc, char *argv[], std::function<void()> outputBanner);
   void commonShutdown();
} // namespace mikado::common

#endif // CMN_GLOBALS_H
