#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_SMART_ENUMS_H)
#define CMN_SMART_ENUMS_H

// This is the "smart" enum library from https://github.com/BlackMATov/enum.hpp/
#include "../../../enum.hpp/headers/enum.hpp/enum.hpp"

//////////////////////////////////////////////////////////////////////////////
// Add traits to the ixWebsocket library's enums that we use
namespace ix {

   ENUM_HPP_TRAITS_DECL(WebSocketMessageType,
      (Message = 0)
      (Open = 1)
      (Close = 2)
      (Error = 3)
      (Ping = 4)
      (Pong = 5)
      (Fragment = 6)
   )
   ENUM_HPP_REGISTER_TRAITS(WebSocketMessageType)
   std::ostream &operator<<(std::ostream &os, WebSocketMessageType code);

} // namespace ix

#endif // CMN_SMART_ENUMS_H
