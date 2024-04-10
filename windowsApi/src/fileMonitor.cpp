// Copyright (c) Gamaliel Ltd

#include "common.h"
#include "windowsApi/fileMonitor.h"
#include "windowsApi/globals.h"
#include "windowsApi/handle.h"

namespace json = boost::json;
using namespace boost::tuples;
using namespace std;
using namespace filesystem;

namespace mikado::windowsApi {
   
   typedef mikado::common::MikadoErrorCode MikadoErrorCode; // To reduce typing

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsFileMonitor::WindowsFileMonitor(UpdateQueuePtr updateQueue)
      : updateQueue_(updateQueue)
      , rootEvent_(CreateEvent(NULL, TRUE, FALSE, NULL), "WindowsFileMonitor") {
   }

   ///////////////////////////////////////////////////////////////////////
   //
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

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsFileMonitor::addMonitor(Action action, path const &folder
         , path const &filename, MonitorId *id) {

      MonitorType monitor { action, move(common::lexicalPath(folder)), move(common::lexicalPath(filename)) };
      actions_.emplace_back(move(monitor));
      if (id) {
         *id = actions_.end();
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsFileMonitor::removeMonitor(MonitorId id) {
      actions_.erase(id);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   bool WindowsFileMonitor::stopRequested() const {
      return isRunning_;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsFileMonitor::stopMonitoring() {
      // Tell the monitor thread to shut down
      isRunning_ = false;

      // Kick the event to stop reading directory changes
      if (!SetEvent((HANDLE)rootEvent_)) {
         str_error() << "Could not signal the event to stop monitoring: "
            << getLastErrorAsString() << endl;
         return MikadoErrorCode::MKO_ERROR_INVALID_CONFIG;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsFileMonitor::processDirectoryChange(json::array *jv
         , FILE_NOTIFY_INFORMATION const *notification) {

      string actionString;
      path pathName{ common::toString(wstring(notification->FileName, notification->FileNameLength / sizeof(wchar_t))) };

      // Output the file details
      str_info() << "Detected a change to: " << pathName << endl;

      switch (notification->Action)
      {
      case FILE_ACTION_ADDED:
         actionString = "added";
         break;
      case FILE_ACTION_REMOVED:
         actionString = "removed";
         break;
      case FILE_ACTION_MODIFIED:
         actionString = "modified";
         break;
      case FILE_ACTION_RENAMED_OLD_NAME:
         actionString = "renameFrom";
         break;
      case FILE_ACTION_RENAMED_NEW_NAME:
         actionString = "renameTo";
         break;

      default:
         str_error() << "unrecognized directory change action: " << notification->Action << endl;
         return MikadoErrorCode::MKO_STATUS_UNKNOWN_CODE;
      }
      jv->emplace_back(
         json::object {
            { "action", actionString },
            { "path", pathName.string() }
         });
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsFileMonitor::run(path const &rootFolder) {
      WindowsHandlePtr folderHandle;
      try
      {
         isRunning_ = true;

         // Register the folder
         auto folderNameW = rootFolder.wstring();
         folderHandle = make_shared<WindowsHandle>(
            CreateFileW(folderNameW.c_str(), FILE_LIST_DIRECTORY
               , FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
               NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL), "WindowsFileMonitor");
         if (!folderHandle->isOpen()) {
            str_error() << "Could not open directory " << rootFolder << ": " << getLastErrorAsString() << endl;
            return MikadoErrorCode::MKO_ERROR_INVALID_CONFIG;
         }

         str_info() << "monitoring filters from " << rootFolder << endl;
         for (auto const &monitor : actions_) {
            str_info() << "monitoring " << monitor.get<1>() << " for " << monitor.get<2>() << endl;
         }

         // Monitor the root folder for changes

         DWORD buffer[32 * 1024];
         DWORD bytesReturned;
         DWORD const watchFlags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE
            | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
         while (isRunning_) {
            
            // Use a new OVERLAPPED structure instance for each call to ReadDirectoryChangesW()
            OVERLAPPED overlapped;
            ZeroMemory(&overlapped, sizeof(overlapped));
            overlapped.hEvent = rootEvent_;

            // Prime the directory watcher with overlapped I/O
            if (!ReadDirectoryChangesW(*folderHandle, buffer, sizeof(buffer), TRUE, watchFlags, NULL, &overlapped, NULL)) {
               isRunning_ = false;
               str_error() << "Error reading directory changes: " << getLastErrorAsString() << endl;
               return MikadoErrorCode::MKO_ERROR_MONITOR_FAILED;
            }

            // Wait for a change to a file in or under the root folder
            if (isRunning_ && !GetOverlappedResultEx(*folderHandle, &overlapped, &bytesReturned, INFINITE, TRUE)) {               
               str_error() << "Error getting overlapped result: " << getLastErrorAsString() << endl;
               isRunning_ = false;
               return MikadoErrorCode::MKO_ERROR_MONITOR_FAILED;
            }

            if (0 == bytesReturned) {
               if (isRunning_) {
                  // Too many changes detected.
                  str_warn() << "ReadDirectoryChangesW() returned 0 bytes; too many changes?" << endl;
               }
               continue;
            }

            // Process all of the updates 
            json::array jv;
            auto offset = 0;
            auto notification = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(buffer);
            do {
               notification = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(reinterpret_cast<char *>(notification) + offset);
               if (auto rc = processDirectoryChange(&jv, notification); MikadoErrorCode::MKO_ERROR_NONE != rc) {
                  if (MKO_IS_ERROR(rc)) {
                     return rc;
                  }
                  // Otherwise, we just ignore the error and keep going
               }  

               offset += notification->NextEntryOffset;
            } while (notification->NextEntryOffset);

            auto update = make_shared<QueueData>();
            update->json = serialize(jv);
            updateQueue_->push(update);
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
