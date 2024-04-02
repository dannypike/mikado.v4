#include "common.h"

namespace blf = boost::lockfree;
namespace json = boost::json;
namespace common = mikado::common;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;
using namespace std::filesystem;

// Simulate a connection from another application
ix::WebSocket WSS;

namespace mikado::common {

   static void processIncoming(ix::WebSocketMessagePtr const &msg) {
      if (msg->type == ix::WebSocketMessageType::Open) {
         str_info() << "Connected to " << msg->openInfo.uri << endl;
      }
      else if (msg->type == ix::WebSocketMessageType::Close) {
         str_info() << "Disconnected from " << msg->closeInfo.reason << endl;
      }
      else if (msg->type == ix::WebSocketMessageType::Message) {
         str_info() << "Received message: " << msg->str << endl;
      }
      else if (msg->type == ix::WebSocketMessageType::Error) {
         str_error() << "Error: " << msg->errorInfo.reason << endl;
      }
   }

   typedef blf::spsc_queue<string, blf::capacity<1024>> TestQueue;
   TestQueue SendingQueue;

   void testConnect(common::ConfigurePtr options) {

      auto port = options->get<int>(common::kBrokerPort);
      auto host = options->get<string>(common::kBrokerHost);
      stringstream url;
      url << "ws://" << host << ":" << port << "/";
      url.flush();

      WSS.setUrl(url.str());
      WSS.setPingInterval(30);
      WSS.disablePerMessageDeflate();
      WSS.setOnMessageCallback(processIncoming);

      str_info() << "Connecting to " << WSS.getUrl() << endl;
      WSS.start();

      json::value jv = { {
            // The header
            {common::fieldVersion, MKO_WEBSOCKET_PROTOCOL_VERSION },
            {common::fieldType, common::actionConnect},
            {common::fieldAppId, common::appIdGlobber},
            {common::fieldTimestamp, common::formatTimestamp()}
         }, {
            // The data
            {common::fieldAction, common::actionConnect},
         }
      };
      auto text = serialize(jv);

      str_info() << "sign-on message will be: " << text << endl;
      SendingQueue.push(text);
   }

   bool testProcess() {
      if (WSS.getReadyState() == ix::ReadyState::Open) {
         return 0 < SendingQueue.consume_all([](string &text) {
            str_info() << "sending: '" << text << "'" << endl;
            WSS.send(text);
         });
      }
      return false;
   }

   void testShutdown() {
      if (WSS.getReadyState() == ix::ReadyState::Open) {
         WSS.stop();
      }
   }

} // namespace mikado::common
