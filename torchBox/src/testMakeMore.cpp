#include "common.h"
#include "torchBox/testMakeMore.h"

namespace bt = boost::posix_time;
namespace common = mikado::common;
using namespace std;

namespace mikado::torchBox {

   void TestMakeMore::run() {
      torch::Tensor tensor;
      auto startedAt = bt::second_clock::local_time();
      str_info() << "Running test 'MakeMore'" << endl;

      auto elapsed = bt::second_clock::local_time() - startedAt;

      str_info() << "MakeMore took " << elapsed << " seconds." << endl;
   }

} // namespace mikado::torchBox
