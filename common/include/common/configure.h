#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_CONFIGURE_H)
#define CMN_CONFIGURE_H

namespace mikado::common {

   class Configure : public std::enable_shared_from_this<Configure> {
   public:
      Configure(std::string const &appId, std::string const &consoleTitle);
      ~Configure() = default;

      boost::program_options::options_description_easy_init addOptions();
      MikadoErrorCode importFile(std::filesystem::path const &cfgFilename);
      MikadoErrorCode importCommandline(int argc, char *argv[]);
      MikadoErrorCode notify();

      // Many of the applications behave the same when when scanning options
      MikadoErrorCode defaultProcessing(int argc, char *argv[]
         , std::function<void()> showBanner);

      boost::program_options::options_description const &description() const {
         return *options_;
      }

      bool hasOption(std::string const &name) const {
         return values_->count(name) > 0;
      }

      template <class TT> TT get(std::string const &name) const {
         if (!hasOption(name)) {
            return TT{};
         }
         return (*values_)[name].as<TT>();
      }

      template <class TT> TT get(std::string const &name, TT const &defaultValue) const {
         if (!hasOption(name)) {
            return defaultValue;
         }
         return (*values_)[name].as<TT>();
      }

   protected:
      MikadoErrorCode checkBroker();
      MikadoErrorCode showHelp(std::string const &exeName, std::function<void()> showBanner);
      static BOOL WINAPI consoleCtrlHandler(DWORD ctrlType);

   private:
      std::string const appId_;  // The application "name" that is used when communicating over the websockets

      typedef std::shared_ptr<boost::program_options::options_description> OptionsPtr;
      typedef std::shared_ptr<boost::program_options::variables_map> VariablesPtr;
      typedef std::shared_ptr<ix::WebSocket> WebSocketPtr;
      OptionsPtr options_;
      VariablesPtr values_;

      std::string brokerHost_ { "127.0.0.1" };
      int brokerPort_ = 22304;
      unsigned brokerTimeout_ = 2;
      WebSocketPtr broker_;
   };
   typedef std::shared_ptr<Configure> ConfigurePtr;

} // namespace mikado::common

#endif // CMN_CONFIGURE_H
