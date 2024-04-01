#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(BRK_APPID_H)
#define BRK_APPID_H

namespace mikado::broker {

   // Typesafe wrapper for an application identifier

   class AppId : private std::string {
   public:
      AppId(std::string const &appId) : std::string(appId) {
      }
      AppId(AppId const &appId) : std::string(appId) {
      }
      AppId(AppId &&appId) : std::string(std::move(appId)) {
      }
      AppId &operator=(AppId const &appId) {
         std::string::operator=(appId);
         return *this;
      }
      AppId &operator=(AppId &&appId) {
         std::string::operator=(std::move(appId));
         return *this;
      }
      bool operator==(AppId const &other) const {
         // AppIds are case-insensitive
         return boost::iequals(static_cast<std::string const &>(*this), static_cast<std::string const &>(other));
      }
      bool operator!=(AppId const &other) const {
         return !(*this == other);
      }
      bool operator<(AppId const &other) const {
         return boost::ilexicographical_compare(static_cast<std::string const &>(*this), static_cast<std::string const &>(other));
      }
      std::string const &toString() const {
         return static_cast<std::string const &>(*this);
      }
   };
   typedef std::shared_ptr<AppId> AppIdPtr;
   std::ostream &operator<<(std::ostream &os, AppId const &appId);   // defined in broker.cpp

   // Hasher for AppInstanceId, so we can use it in a std::map
   class AppIdHasher {
   public:
      // Functions to allow this to be used as the key to a map:
      std::size_t operator()(const AppId &asKey) const {
         // It's a string, so just use the hash of the string
         return std::hash<std::string>()(asKey.toString());
      }
   };

} // namespace mikado::broker

#endif // BRK_APPID_H
