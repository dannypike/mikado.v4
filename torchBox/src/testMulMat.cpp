#include "common.h"
#include "torchBox/testMulMat.h"

namespace bt = boost::posix_time;
namespace common = mikado::common;
using namespace std;

namespace mikado::torchBox {

   void TestMulMat::run() {
      torch::Tensor tensor;
      auto startedAt = bt::second_clock::local_time();
      auto count = 16384;
      auto dims = 1024;
      str_info() << "Running test 'MulMat' with " << count
         << " iterations of a matrix [" << dims << "x" << dims
         << "]" << endl;

      for (auto ii = 0; ii < count; ++ii) {
         torch::Tensor tensor1 = torch::rand({ dims, dims }, getC10Device());
         torch::Tensor tensor2 = torch::rand({ dims, dims }, getC10Device());
         tensor = torch::mm(tensor1, tensor2);
      }
      auto elapsed = bt::second_clock::local_time() - startedAt;

      str_info() << "The product of the two tensors is loaded into '"
         << enum_hpp::to_string(tensor.device().type()).value_or("???") << "':" << endl
         << "The calculations took " << elapsed << " seconds." << endl
         ; // << tensor << endl;
   }

} // namespace mikado::torchBox