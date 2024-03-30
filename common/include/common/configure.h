#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_CONFIGURE_H)
#define CMN_CONFIGURE_H

namespace mikado::common {

   class Configure : public std::enable_shared_from_this<Configure> {
   public:
      Configure(std::string const &appId, std::string const &consoleTitle
         , BrokerFYI *fyi);
      ~Configure() = default;

      WebSocketPtr broker() const {
         return brokerWS_;
      }

      boost::program_options::options_description_easy_init addOptions() {
         return options_->add_options();
      }
      MikadoErrorCode importFile(std::filesystem::path const &cfgFilename);
      MikadoErrorCode importCommandline(int argc, char *argv[]);
      MikadoErrorCode notify();

      // Many of the applications behave the same when when scanning options
      MikadoErrorCode defaultProcessing(int argc, char *argv[]
         , std::function<void()> showBanner);

      boost::program_options::options_description const &description() const {
         return *options_;
      }

      bool has(std::string const &name) const {
         return values_->count(name) > 0;
      }

      template <class TT> TT get(std::string const &name) const {
         if (!has(name)) {
            return TT{};
         }
         return (*values_)[name].as<TT>();
      }

      template <class TT> TT get(std::string const &name, TT const &defaultValue) const {
         if (!has(name)) {
            return defaultValue;
         }
         return (*values_)[name].as<TT>();
      }

      template <class TT> bool tryGet(std::string const &name, TT &value) const {
         auto hasValue = has(name); 
         if (hasValue) {
            value = (*values_)[name].as<TT>();
         }
         return hasValue;
      }

      template <class TT> bool tryGet(std::string const &name
            , TT const &defaultValue, TT &value) const {
         auto hasValue = has(name);
         value = hasValue ? (*values_)[name].as<TT>() : defaultValue;
         return hasValue;
      }

   protected:
      MikadoErrorCode processBrokerMessage(ix::WebSocketMessagePtr const &msg);
      MikadoErrorCode checkBroker();
      MikadoErrorCode showHelp(std::string const &exeName, std::function<void()> showBanner);
      static BOOL WINAPI consoleCtrlHandler(DWORD ctrlType);

   private:
      std::string const appId_;  // The application "name" that is used when communicating over the websockets

      typedef std::shared_ptr<boost::program_options::options_description> OptionsPtr;
      typedef std::shared_ptr<boost::program_options::variables_map> VariablesPtr;
      OptionsPtr options_;
      VariablesPtr values_;

      std::string brokerHost_ { "127.0.0.1" };
      int brokerPort_ = 22304;
      unsigned brokerTimeout_ = 2;
      WebSocketPtr brokerWS_;
   };
   typedef std::shared_ptr<Configure> ConfigurePtr;

} // namespace mikado::common

#endif // CMN_CONFIGURE_H
