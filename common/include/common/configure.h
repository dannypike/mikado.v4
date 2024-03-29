#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_CONFIGURE_H)
#define CMN_CONFIGURE_H

namespace mikado::common {

   class Configure : public std::enable_shared_from_this<Configure> {
   public:
      Configure() = default;
      ~Configure() = default;

      boost::program_options::options_description_easy_init addOptions();
      MikadoErrorCode importFile(std::filesystem::path const &cfgFilename);
      MikadoErrorCode importCommandline(int argc, char *argv[]);
      MikadoErrorCode notify();

      boost::program_options::options_description const &description() const {
         return options_;
      }

      bool hasOption(std::string const &name) const {
         return values_.count(name) > 0;
      }

      template <class TT> TT get(std::string const &name) const {
         if (!hasOption(name)) {
            return TT{};
         }
         return values_[name].as<TT>();
      }

      template <class TT> TT get(std::string const &name, TT const &defaultValue) const {
         if (!hasOption(name)) {
            return defaultValue;
         }
         return values_[name].as<TT>();
      }

   private: 
      boost::program_options::options_description options_;
      boost::program_options::variables_map values_;
   };
   typedef std::shared_ptr<Configure> ConfigurePtr;

} // namespace mikado::common

#endif // CMN_CONFIGURE_H
