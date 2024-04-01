// Copyright (c) 2024 Gamaliel Ltd
#include "common.h"
#include "windowsApi.h"
#include "broker/appManager.h"

namespace api = mikado::windowsApi;
namespace common = mikado::common;
namespace guids = boost::uuids;
namespace po = boost::program_options;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;

namespace mikado::broker {

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode AppManager::configureAppManager(common::ConfigurePtr cfg) {
      return MikadoErrorCode::MKO_ERROR_NOT_IMPLEMENTED;
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
   MikadoErrorCode AppManager::dropAppInstance(AppId const &appId, AppInstanceId const &instanceId) {
      if (auto it = appStore_.find(appId.toString()); it != appStore_.end()) {
         InstanceStorePtr appInstances = it->second;
         auto it2 = appInstances->find(instanceId.toString());
         if (it2 != appInstances->end()) {
            AppPtr app = it2->second;
            appInstances->erase(it2);
            str_info() << "Stopping app '" << appId << "' with instanceId '" << instanceId;
            app->stop();

            if (appInstances->empty()) {
               appStore_.erase(it);
               str_info() << "App '" << appId << "' has no more instances, removing from store";
            }
            return MikadoErrorCode::MKO_ERROR_NONE;
         }
      }
      str_warn() << "App '" << appId << "' with instanceId '" << instanceId << "' not found";
      return MikadoErrorCode::MKO_STATUS_NOOP;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode AppManager::runAppManager() {
      return MikadoErrorCode::MKO_ERROR_NOT_IMPLEMENTED;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void AppManager::shutdownAppManager() {
   }

} // namespace mikado::broker
