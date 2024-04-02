// Copyright (c) 2024 Gamaliel Ltd
#include "common.h"
#include "windowsApi.h"
#include "broker/appManager.h"

namespace api = mikado::windowsApi;
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
      auto appIndex = 0;
      for (auto const &app : cfg->get<vector<string>>(common::kStartApp)) {
         try
         {
            ++appIndex;

            error_code ec;
            json::value jv = json::parse(app, ec);
            if (ec) {
               str_error() << "Error parsing app definition #" << appIndex << ": " << ec.message() << endl;
               return MikadoErrorCode::MKO_ERROR_APP_CONFIGURE;
            }
            string appId{ jv.at(common::kAppId).as_string().c_str() };
            str_info() << "Configuring app '" << appId << "'" << endl;

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
            }

            // Extract the arguments for the app
            vector<string> args;
            if (auto rc = common::jsonVectorString(args, jv, common::kArgs.c_str()); MKO_IS_ERROR(rc)) {
               str_error() << "Error parsing app '" << appId << "' arguments: " << rc << endl;
               return rc;
            }

            // Construct the propcess object that will manage the running app
            auto process = make_shared<api::WindowsProcess>(appId.c_str());
            process->setFolder(startFolder);
            process->setExecutable(exePath);
            for (auto const &arg : args) {
               process->addArgument(arg);
            }

            // Allocate an InstanceId for this app
            auto app = addAppInstance(appId);

            // And store the process in it
            app->setProcess(process);
         }
         catch (const std::exception &e)
         {
            log_exception(e);
            return MikadoErrorCode::MKO_ERROR_APP_CONFIGURE;
         }
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   AppPtr AppManager::addAppInstance(AppId const &appId) {
      
      InstanceStorePtr appInstances;
      if (auto it = appStore_.find(appId.toString()); it != appStore_.end()) {
         appInstances = it->second;
      }
      else {
         appInstances = make_shared<InstanceStore>();
      }

      guids::random_generator generator;
      guids::uuid uuid = generator();
      AppInstanceId instanceId { boost::lexical_cast<std::string>(uuid) };
      auto app = make_shared<App>(appId, instanceId);
      appInstances->insert(make_pair(instanceId.toString(), app));
      str_info() << "Created new app '" << appId << "' with instanceId '" << instanceId;
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

      // Make sure that each of the apps that we need to start is running
      auto startableApps = vector<AppPtr>{};
      for (auto const &[appId, appInstances] : appStore_) {
         for (auto const &[instanceId, app] : *appInstances) {
            auto process = app->getProcess();
            if (process && !process->isRunning() && !process->startFailed()) {
               startableApps.push_back(app);
            }
         }
      }

      if (startableApps.empty()) {
         static bool firstTime = true;
         if (firstTime) {
            str_info() << "No apps to start" << endl;
            firstTime = false;
         }
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
               continue;
            }
            if (process->startFailed()) {
               str_error() << "App '" << app->appId() << "' failed to start" << endl;
               return MikadoErrorCode::MKO_ERROR_APP_START;
            }

            // Start the app
            if (auto rc = process->run(); MKO_IS_ERROR(rc)) {
               str_error() << "Error starting app '" << app->appId() << "': " << rc << endl;
               return rc;
            }
         }
         catch (const std::exception &e)
         {
            log_exception(e);
            return MikadoErrorCode::MKO_ERROR_APP_START;
         }
      }

      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void AppManager::shutdownAppManager() {
   }

} // namespace mikado::broker
