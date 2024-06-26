#include "common.h"
#include "torchBox/netMulMat.h"

namespace bt = boost::posix_time;
namespace common = mikado::common;
using namespace std;

namespace mikado::torchBox {

   typedef common::MikadoErrorCode MikadoErrorCode;

   ///////////////////////////////////////////////////////////////////////////
   //
   NetMulMat::NetMulMat()
      : NetBase(common::kMulMat) {
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMulMat::configure(common::ConfigurePtr cfg) {
      return MikadoErrorCode::MKO_STATUS_NOOP;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode NetMulMat::train() {
      torch::Tensor tensor;
      auto startedAt = bt::second_clock::local_time();
      auto count = 16384;
      auto dims = 1024;
      str_info() << "training '" << getName() << "' with " << count
         << " iterations of a matrix [" << dims << "x" << dims
         << "]" << endl;

      auto device = getC10Device();
      for (auto ii = 0; ii < count; ++ii) {
         torch::Tensor tensor1 = torch::rand({ dims, dims }, device);
         torch::Tensor tensor2 = torch::rand({ dims, dims }, device);
         tensor = torch::mm(tensor1, tensor2);
      }
      auto elapsed = bt::second_clock::local_time() - startedAt;

      auto deviceType = tensor.device().type();
      str_info() << "The product of the two tensors is loaded into '"
         << enum_hpp::to_string(deviceType).value_or<std::string>(common::unknownEnumAsString((int)deviceType))
         << "':" << endl
         << "The calculations took " << elapsed << " seconds." << endl; // << tensor << endl;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

} // namespace mikado::torchBox