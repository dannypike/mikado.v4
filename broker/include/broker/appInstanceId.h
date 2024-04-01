#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(BRK_APP_INSTANCE_ID_H)
#define BRK_APP_INSTANCE_ID_H

namespace mikado::broker {

   // Typesafe wrapper for an application identifier

   class AppInstanceId : public std::string {
   public:
      AppInstanceId(std::string const &appId) : std::string(appId) {
      }
      AppInstanceId(char const *appId) : std::string(appId) {
      }
      AppInstanceId(AppInstanceId const &appId) : std::string(appId) {
      }
      AppInstanceId(AppInstanceId &&appId) : std::string(std::move(appId)) {
      }
      AppInstanceId &operator=(AppInstanceId const &appId) {
         std::string::operator=(appId);
         return *this;
      }
      AppInstanceId &operator=(AppInstanceId &&appId) {
         std::string::operator=(std::move(appId));
         return *this;
      }
      bool operator==(AppInstanceId const &other) const {
         // AppInstanceIds are case-insensitive
         return boost::iequals(static_cast<std::string const &>(*this), static_cast<std::string const &>(other));
      }
      bool operator!=(AppInstanceId const &other) const {
         return !(*this == other);
      }
      bool operator<(AppInstanceId const &other) const {
         return boost::ilexicographical_compare(static_cast<std::string const &>(*this), static_cast<std::string const &>(other));
      }
      std::string const &toString() const {
         return *this;
      }
   };
   typedef std::shared_ptr<AppInstanceId> AppInstanceIdPtr;
   std::ostream &operator<<(std::ostream &os, AppInstanceId const &appInstanceId);   // defined in broker.cpp

   // Hasher for AppInstanceId, so we can use it in a std::map
   class AppInstanceIdHasher {
   public:
      // Functions to allow this to be used as the key to a map:
      std::size_t operator()(const AppInstanceId &asKey) const {
         // It's a string, so just use the hash of the string
         return std::hash<std::string>()(asKey);
      }
   };

} // namespace mikado::broker

#endif // BRK_APP_INSTANCE_ID_H
