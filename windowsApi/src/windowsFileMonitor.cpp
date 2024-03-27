// Copyright (c) Gamaliel Ltd
#include "common/algorithms.h"
#include "common/errorCodes.h"
#include "common/logger.h"
#include "windowsApi/windowsFileMonitor.h"
#include "windowsApi/windowsHandle.h"

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

#include <windows.h>
#include <iostream>

   void MonitorDirectory(LPCWSTR path) {
      DWORD buffer[1024];
      DWORD bytesReturned;
      HANDLE directory = CreateFileW(path, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
         NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

      if (directory == INVALID_HANDLE_VALUE) {
         std::cerr << "Could not open directory.\n";
         return;
      }

      while (true) {
         if (ReadDirectoryChangesW(directory, buffer, sizeof(buffer), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME, &bytesReturned, NULL, NULL)) {
            FILE_NOTIFY_INFORMATION *notification = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(buffer);

            // Print the name of the changed file
            std::wcout << "File changed: " << std::wstring(notification->FileName, notification->FileNameLength / sizeof(wchar_t)) << '\n';
         }
      }

      CloseHandle(directory);
   }

   int main() {
      MonitorDirectory(L"C:\\path\\to\\directory");
      return 0;
   }

   MikadoErrorCode WindowsFileMonitor::run(path const &rootFolder, bool *stopMonitoring) {
      WindowsHandlePtr folderHandle;
      try
      {
         // Register the folder
         auto folderNameW = rootFolder.wstring();
         folderHandle = make_shared<WindowsHandle>(
            CreateFileW(folderNameW.c_str(), FILE_LIST_DIRECTORY
               , FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
               NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL), "WindowsFileMonitor");
         if (!folderHandle->isOpen()) {
            str_error() << "Could not open directory " << rootFolder << endl;
            return MikadoErrorCode::MKO_ERROR_INVALID_CONFIG;
         }

         str_info() << "monitoring filters from " << rootFolder << endl;
         for (auto const &monitor : actions_) {
            str_info() << "monitoring " << monitor.get<1>() << " for " << monitor.get<2>() << endl;
         }

         // Monitor the root folder for changes
         DWORD buffer[1024];
         DWORD bytesReturned;
         while (!*stopMonitoring) {
            if (ReadDirectoryChangesW(*folderHandle, buffer, sizeof(buffer), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME, &bytesReturned, NULL, NULL)) {
               FILE_NOTIFY_INFORMATION *notification = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(buffer);
               path pathName { common::toString(wstring(notification->FileName, notification->FileNameLength / sizeof(wchar_t))) };

               // Print the name of the changed file
               str_info() << "Detected a change to: " << pathName << endl;
            }
         }
      }
      catch (const std::exception &e)
      {
         log_exception(e);
      }

      str_info() << "detected request to stop monitoring" << endl;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

} // namespace mikado::windowsApi
