#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(BRK_APP_MANAGER_H)
#define BRK_APP_MANAGER_H

#include "broker/app.h"

namespace mikado::broker {

   class AppManager : public std::enable_shared_from_this<AppManager> {
      typedef common::MikadoErrorCode MikadoErrorCode;

   public:
      MikadoErrorCode configureAppManager(common::ConfigurePtr cfg);
      AppPtr addAppInstance(AppId const &appId);
      MikadoErrorCode dropAppInstance(AppPtr app);
      MikadoErrorCode dropAppInstance(AppId const &appId, AppInstanceId const &instanceId);
      MikadoErrorCode runAppManager(common::ConfigurePtr options);
      void shutdownAppManager();

   protected:
      typedef std::unordered_map<AppInstanceId, AppPtr, AppInstanceIdHasher> InstanceStore;
      typedef std::shared_ptr<InstanceStore> InstanceStorePtr;
      typedef std::unordered_map<AppId, InstanceStorePtr, AppIdHasher> AppStore;

   private:
      AppStore appStore_;
   };
   typedef std::shared_ptr<AppManager> AppManagerPtr;

} // namespace mikado::broker

#endif // BRK_APP_MANAGER_H
