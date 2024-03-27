#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(API_WINDOWS_FILEMON_H)
#define API_WINDOWS_FILEMON_H

#include "common/errorCodes.h"

namespace mikado::windowsApi {

   class WindowsFileMonitor {
      typedef mikado::common::MikadoErrorCode MikadoErrorCode; // To reduce typing

   public:
      enum class Action { None, Include, Exclude };

      WindowsFileMonitor() = default;
      virtual ~WindowsFileMonitor() = default;

      typedef boost::tuple<Action, std::filesystem::path, std::filesystem::path> MonitorType;
      typedef std::list<MonitorType>::iterator MonitorId;

      Action getDefaultAction() const {
         return defaultAction_;
      }
      Action setDefaultAction(Action theValue) {
         std::swap(defaultAction_, theValue);
         return theValue;
      }

      // Only need to supply an id parameter if you want to be able to delete a monitor later
      MikadoErrorCode includeGlob(std::filesystem::path glob, MonitorId *id = nullptr) {
         return addMonitor(Action::Include, glob, id);
      }
      MikadoErrorCode excludeGlob(std::filesystem::path glob, MonitorId *id = nullptr) {
         return addMonitor(Action::Exclude, glob, id);
      }
      MikadoErrorCode removeMonitor(MonitorId id);

      MikadoErrorCode run(std::filesystem::path const &rootFolder, bool *stopMonitoring = nullptr);

   protected:
      MikadoErrorCode addMonitor(Action action, std::filesystem::path const &glob, MonitorId *id = nullptr);
      MikadoErrorCode addMonitor(Action action, std::filesystem::path const &folder, std::filesystem::path const &fileGlob
         , MonitorId *id = nullptr);

   private:
      Action defaultAction_ = Action::Exclude;
      std::list<MonitorType> actions_;
   };

   typedef std::shared_ptr<WindowsFileMonitor> WindowsFileMonitorPtr;

} // namespace mikado::windowsApi

#endif // API_WINDOWS_FILEMON_H
