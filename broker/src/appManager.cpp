// Copyright (c) 2024 Gamaliel Ltd
#include "common.h"
#include "windowsApi.h"
#include "broker/appManager.h"

namespace api = mikado::windowsApi;
namespace bt = boost::posix_time;
namespace common = mikado::common;
namespace guids = boost::uuids;
namespace json = boost::json;
namespace po = boost::program_options;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;
using namespace filesystem;

namespace mikado::broker {

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode AppManager::configureAppManager(common::ConfigurePtr cfg) {
      auto appIndex = 0;   // for debug messages
      for (auto const &app : cfg->get<vector<string>>(common::kStartApp)) {
         try
         {
            ++appIndex;

            error_code ec;
            json::value jv = json::parse(app, ec);
            if (ec || !jv.is_object()) {
               auto error = ec ? ec.message() : "not an object";
               str_error() << "Error parsing app definition #" << appIndex << ": " << error << endl;
               return MikadoErrorCode::MKO_ERROR_APP_CONFIGURE;
            }

            json::object jo = jv.as_object();
            string appId{ jo.at(common::kAppId).as_string().c_str() };

            // If there is an instanceId specified, then we use that and wait for the app to be started
            // manually (this is for debugging apps). Otherwise, we generate one and start the app
            // ourselves (this is for normal operation).
            if (jo.contains(common::kInstanceId)) {
               // Debugging where we want to start the apps manually and have the broker wait for them
               string instanceIdStr { jo.at(common::kInstanceId).as_string().c_str() };
               str_info() << "adding a placeholder for app '" << appId << "' with instanceId '"
                  << instanceIdStr << "', to be started separately" << endl;
               auto app = addAppInstance(appId, instanceIdStr.c_str());
            }
            else {
               // Normal operation, where the broker manages the lifetimes of the apps.
               str_info() << "Configuring app '" << appId << "', to be started by the Broker" << endl;
               if (auto rc = configureStartup(jo, appId); MKO_IS_ERROR(rc)) {
                  return rc;
               }
            }
         }
         catch (const exception &e)
         {
            log_exception(e);
            return MikadoErrorCode::MKO_ERROR_APP_CONFIGURE;
         }
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode AppManager::configureComspec(json::value const &jv, string const &appId
         , vector<string> &args, path &exePath) {

      vector<string> argsComspec;
      if (jv.is_object() && jv.as_object().contains(common::kStartComspec.c_str())) {
         if (auto rc = common::jsonVectorString(argsComspec, jv
               , common::kStartComspec.c_str()); MKO_IS_ERROR(rc)) {
            return rc;
         }
      }
      if (argsComspec.empty()) {
         return MikadoErrorCode::MKO_STATUS_NOOP;
      }

      path cmdPath{ common::lexicalPath(getenv("COMSPEC")) };
      if (cmdPath.empty() || !exists(cmdPath)) {
         str_error() << "App '" << appId << "' cannot run under Command Prompt, as the COMSPEC "
            "environment variable is invalid / does not exist: " << cmdPath << endl;
         return MikadoErrorCode::MKO_ERROR_APP_COMSPEC;
      }

      // Insert the COMSPEC arguments
      str_debug() << "App '" << appId << "' will run inside a Command Prompt configured with "
         << argsComspec.size() << " argument(s): " << endl;
      for (auto aa : argsComspec) {
         str_debug() << "  \"" << aa << "\"" << endl;
         args.emplace_back(aa);
      }

      // replace the exe name with the Command Prompt exe
      args.emplace_back(exePath.string());
      exePath = move(cmdPath);
      
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode AppManager::configureStartup(json::value const &jv, string const &appId) {

      vector<string> args;

      // The exe pathname is either absolute or it is relative to the Broker exe
      path exePath{ move(common::lexicalPath(common::jsonPropertyString(jv, common::kExePath).c_str())) };
      if (!exists(exePath)) {
         str_error() << "App '" << appId << "' exe not found: " << exePath << endl;
         return MikadoErrorCode::MKO_ERROR_APP_CONFIGURE;
      }

      // The "current folder" for the app is relative ot the app's folder, not the Broker's current folder
      // and not relative to the Broker exe (unless that's the same as the app's exe folder).
      path startFolder{ move(common::lexicalPath(common::jsonPropertyString(jv, common::kStartFolder).c_str())) };
      if (!exists(startFolder)) {
         if (!create_directories(startFolder)) {
            str_error() << "App '" << appId << "' start folder not found: "
               << startFolder << ", and cannot be created" << endl;
            return MikadoErrorCode::MKO_ERROR_APP_CONFIGURE;
         }
         str_debug() << "created folder " << startFolder << " for app '" << appId << "'" << endl;
      }

      // Should we run the app inside a Command Prompt?
      if (auto rc = configureComspec(jv, appId, args, exePath); MKO_IS_ERROR(rc)) {
         return rc;
      }

      // Extract the arguments for the app and add them to the args
      if (auto rc = common::jsonVectorString(args, jv, common::kArgs.c_str()); MKO_IS_ERROR(rc)) {
         str_error() << "Error parsing app '" << appId << "' arguments: " << rc << endl;
         return rc;
      }

      // Allocate an InstanceId for this app
      auto app = addAppInstance(appId);
      args.emplace_back("--" + common::kAppId);
      args.push_back(appId);
      args.emplace_back("--" + common::kInstanceId);
      args.emplace_back(app->instanceId());

      // Construct the propcess object that will manage the running app
      auto process = make_shared<api::WindowsProcess>(appId.c_str());
      process->setFolder(startFolder);
      process->setExecutable(exePath);

      // Transfer the command-line arguments
      for (auto const &arg : args) {
         process->addArgument(arg);
      }

      // And store the process in it
      app->setProcess(process);

      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   AppPtr AppManager::addAppInstance(AppId const &appId, char const *explicitId) {
      
      InstanceStorePtr appInstances;
      if (auto it = appStore_.find(appId.toString()); it != appStore_.end()) {
         appInstances = it->second;
      }
      else {
         appInstances = make_shared<InstanceStore>();
         appStore_.insert(make_pair(appId.toString(), appInstances));
      }

      // The normal case is for us to generate a new instanceId. When debugging the app in
      // Visual Studio, we can specify the instanceId explicitly, so that they match up.
      string instanceIdStr { explicitId ? explicitId : "" };
      if (instanceIdStr.empty()) {
         guids::random_generator generator;
         guids::uuid uuid = generator();
         instanceIdStr = boost::lexical_cast<string>(uuid);
      }

      AppInstanceId instanceId { instanceIdStr };
      auto app = make_shared<App>(appId, instanceId);
      appInstances->insert(make_pair(instanceIdStr, app));
      str_info() << "Created new app '" << appId << "' with instanceId '"
         << instanceId << "'" << endl;
      return app;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode AppManager::dropAppInstance(AppPtr app) {
      auto appId = app->appId();
      auto instanceId = app->instanceId();
      return dropAppInstance(appId, instanceId);
}

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode AppManager::dropAppInstance(AppId const &appId
         , AppInstanceId const &instanceId) {

      if (auto it = appStore_.find(appId.toString()); it != appStore_.end()) {
         InstanceStorePtr appInstances = it->second;
         auto it2 = appInstances->find(instanceId.toString());
         if (it2 != appInstances->end()) {
            auto app = it2->second;

            str_info() << "Stopping app '" << appId << "' with instanceId '" << instanceId << "'" << endl;
            app->stop();

            appInstances->erase(it2);
            if (appInstances->empty()) {
               appStore_.erase(it);
               str_info() << "App '" << appId << "' has no more instances, removing from store" << endl;
            }
            return MikadoErrorCode::MKO_ERROR_NONE;
         }
      }
      str_warn() << "cannot drop unknown app '" << appId << "' with instanceId '" << instanceId << "'" << endl;
      return MikadoErrorCode::MKO_STATUS_NOOP;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode AppManager::runAppManager(common::ConfigurePtr options) {

      auto now = bt::second_clock::universal_time();
      auto retryDelay = options->get<int>(common::kStartRetry);

      // Make sure that each of the apps that we need to start is running
      // Do nothing for apps that will be started manually, e.g. because they are
      // being debugged in Visual Studio.
      auto startableApps = vector<AppPtr>{};
      for (auto const &[appId, appInstances] : appStore_) {
         for (auto const &[instanceId, app] : *appInstances) {
            if (auto process = app->getProcess(); process) {
               // This is an app that we must start
               if (process->startFailed()) {
                  if ((retryDelay >= 0) && (now >= process->startAttemptedAt() + bt::milliseconds(retryDelay))) {
                     // It's OK to try again
                     process->startFailed(false);
                  }
               }
               if (!process->isRunning() && !process->startFailed()) {
                  startableApps.push_back(app);
               }
            }
         }
      }

      if (startableApps.empty()) {
         // There are no apps to be started
         return MikadoErrorCode::MKO_STATUS_NOOP;
      }

      str_info() << "Starting " << startableApps.size() << " apps" << endl;
      for (auto app : startableApps) {
         try
         {
            auto process = app->getProcess();
            if (!process) {
               str_error() << "App '" << app->appId() << "' has no process object" << endl;
               return MikadoErrorCode::MKO_ERROR_APP_START;
            }
            if (process->isRunning()) {
               // already running
               continue;
            }
            if (process->startFailed()) {
               // Already output an error message
               return MikadoErrorCode::MKO_ERROR_APP_START;
            }

            // Start the app
            process->startAttemptedAt(bt::second_clock::universal_time());
            if (auto rc = process->run(); MKO_IS_ERROR(rc)) {
               // Already output an error message
               return rc;
            }
         }
         catch (const exception &e)
         {
            log_exception(e);
            return MikadoErrorCode::MKO_ERROR_APP_START;
         }
      }

      return MikadoErrorCode::MKO_ERROR_NONE;
   }

} // namespace mikado::broker
