#pragma once
#if !defined(CMN_ERROR_CODES_H)
#define CMN_ERROR_CODES_H
// Copyright (c) 2024 Gamaliel Ltd

#include "common/smart_enums.h"

#define MKO_IS_ERROR(code) (MikadoErrorCode::MKO_ERROR_MAXSTATUS < code)
#define MKO_IS_OK(code) (MikadoErrorCode::MKO_ERROR_MAXSTATUS >= code)

namespace mikado::common {

   ENUM_HPP_CLASS_DECL( MikadoErrorCode, int,
      (MKO_ERROR_NONE = 0)
      (MKO_STATUS_EXITOK = 1)
      (MKO_STATUS_ALREADY_DONE = 2)
      (MKO_STATUS_NOT_INITIALIZED = 3)
      (MKO_STATUS_NO_SOURCES = 4)
      (MKO_STATUS_ALREADY_EXISTS = 5)
      (MKO_STATUS_DISABLED = 6)
      (MKO_STATUS_UNEXPECTED = 7)
      (MKO_STATUS_STOPPED = 8)
      (MKO_STATUS_NOOP = 9)
      (MKO_STATUS_HTTP_ERROR = 10)
      (MKO_STATUS_UNKNOWN_CODE = 11)
      (MKO_STATUS_BROKER_AVAILABLE = 12)
      (MKO_STATUS_WAITING = 13)

      (MKO_ERROR_MAXSTATUS = 99)
      (MKO_ERROR_OPEN_FAILED = 100)
      (MKO_ERROR_PATH_NOT_FOUND = 101)
      (MKO_ERROR_INVALID_CONFIG = 102)
      (MKO_ERROR_EXCEPTION = 103)
      (MKO_ERROR_NOT_IMPLEMENTED = 104)
      (MKO_ERROR_NO_CONFIG = 105)
      (MKO_ERROR_INVALID_JSON = 106)
      (MKO_ERROR_WEBSOCKET = 107)
      (MKO_ERROR_CERTIFICATE = 108)
      (MKO_ERROR_BOOST = 109)
      (MKO_ERROR_ALREADY_EXISTS = 110)
      (MKO_ERROR_BAD_MESSAGE = 111)
      (MKO_ERROR_NOT_INITIALIZED = 112)
      (MKO_ERROR_COMMAND_LINE = 113)
      (MKO_ERROR_NO_INPUT = 114)
      (MKO_ERROR_DATABASE = 115)
      (MKO_ERROR_CREATE_FAILED = 116)
      (MKO_ERROR_SQL = 117)
      (MKO_ERROR_TOO_COMPLEX = 118)
      (MKO_ERROR_CREATE_PIPE = 119)
      (MKO_ERROR_SET_HANDLE_INFORMATION = 120)
      (MKO_ERROR_PROCESS_NOT_STARTED = 121)
      (MKO_ERROR_PROCESS_TERMINATED = 122)
      (MKO_ERROR_PROCESS_FAILED = 123)
      (MKO_ERROR_INVALID_ARGUMENTS = 124)
      (MKO_ERROR_DID_NOT_RUN = 125)
      (MKO_ERROR_MONITOR_FAILED = 126)
      (MKO_ERROR_SERVER_LISTEN = 127)
      )

   std::ostream &operator<<(std::ostream &os, enum MikadoErrorCode code);

} // namespace mikado::common

#endif // CMN_ERROR_CODES_H
