#include "common/errorCodes.h"
#include "common/globals.h"
#include "common/logger.h"
#include "windowsApi/windowsGlobals.h"
#include "globber.h"

namespace common = mikado::common;
namespace windowsApi = mikado::windowsApi;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;
using namespace std::filesystem;

namespace mikado::globber {

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode main(int argc, char *argv[]) {
      return MikadoErrorCode::MKO_ERROR_NONE;
   }
    
} // namespace mikado::globber

//////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
    if (auto rc = common::commonInitialize(argc, argv); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }
    if (auto rc = windowsApi::apiInitialize(argc, argv); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }

    int exitCode = (int)mikado::globber::main(argc, argv);
    assert(STATUS_PENDING != exitCode);   // Not allowed to return 259 from any process in Windows, as it is reserved for the system.
    
    windowsApi::apiShutdown();
    common::commonShutdown();
    return exitCode;
}
