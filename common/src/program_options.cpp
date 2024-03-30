// Copyright () 2024 Gamaliel Ltd
#include "common/program_options.h"

using namespace std;

namespace mikado::common {

   // Strings used in boost::program_options descriptions
   string const poBrokerHost { "broker-host" };
   string const poBrokerPort { "broker-port" };
   string const poBrokerTimeout { "broker-timeout" };
   string const poConsoleRestoreOnExit { "console-restore-on-exit" };
   string const poConsoleTitle { "console-title" };
   string const poDefault { "default" };
   string const poExclude { "exclude" };
   string const poHelp { "help" };
   string const poInclude { "include" };
   string const poRoot { "root" };

} // namespace mikado::common