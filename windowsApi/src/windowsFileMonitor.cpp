// Copyright (c) Gamaliel Ltd
#include "common/algorithms.h"
#include "common/errorCodes.h"
#include "common/logger.h"
#include "windowsApi/windowsFileMonitor.h"

using namespace boost::tuples;
using namespace std;
using namespace filesystem;

namespace mikado::windowsApi {
   
   typedef mikado::common::MikadoErrorCode MikadoErrorCode; // To reduce typing

   MikadoErrorCode WindowsFileMonitor::addMonitor(Action action, filesystem::path const &glob, MonitorId *id) {
      auto isWildcard = glob.has_filename() && glob.filename().string().find_first_of("*?") != string::npos;
      if (isWildcard) {
         // This is a wildcard, so we need to monitor the parent folder
         auto parent = glob.parent_path();
         addMonitor(action, parent, glob.filename(), id);
         return MikadoErrorCode::MKO_ERROR_NONE;
      }
      else if (is_directory(glob)) {
         addMonitor(action, glob, "*.*", id);
      }
      else {
         addMonitor(action, current_path(), glob, id);
      }

      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   MikadoErrorCode WindowsFileMonitor::addMonitor(Action action, path const &folder
         , path const &filename, MonitorId *id) {

      MonitorType monitor { action, move(common::lexicalPath(folder)), move(common::lexicalPath(filename)) };
      actions_.emplace_back(move(monitor));
      if (id) {
         *id = actions_.end();
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   MikadoErrorCode WindowsFileMonitor::removeMonitor(MonitorId id) {
      actions_.erase(id);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   MikadoErrorCode WindowsFileMonitor::run(path const &rootFolder, bool *stopMonitoring) {
      str_info() << "monitoring filters from " << rootFolder << endl;
      for (auto const &monitor : actions_) {
         str_info() << "monitoring " << monitor.get<1>() << " for " << monitor.get<2>() << endl;
      }

      while (!*stopMonitoring) {
         // Do the monitoring
      }

      str_info() << "detected request to stop monitoring" << endl;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

} // namespace mikado::windowsApi
