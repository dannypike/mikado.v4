#pragma once
#if !defined(CMN_ASCII_H)
#define CMN_ASCII_H
// Copyright 2024 (c) Gamaliel Ltd

#include <map>

namespace mikado::common {

   class ASCII {
   public:
      ASCII() = delete;
      static constexpr char NUL = 0x00;
      static constexpr char SOH = 0x01;
      static constexpr char STX = 0x02;
      static constexpr char ETX = 0x03;
      static constexpr char EOT = 0x04;
      static constexpr char ENQ = 0x05;
      static constexpr char ACK = 0x06;
      static constexpr char BEL = 0x07;
      static constexpr char BS = 0x08;
      static constexpr char HT = 0x09;
      static constexpr char LF = 0x0A;
      static constexpr char VT = 0x0B;
      static constexpr char FF = 0x0C;
      static constexpr char CR = 0x0D;
      static constexpr char SO = 0x0E;
      static constexpr char SI = 0x0F;
      static constexpr char DLE = 0x10;
      static constexpr char DC1 = 0x11;
      static constexpr char DC2 = 0x12;
      static constexpr char DC3 = 0x13;
      static constexpr char DC4 = 0x14;
      static constexpr char NAK = 0x15;
      static constexpr char SYN = 0x16;
      static constexpr char ETB = 0x17;
      static constexpr char CAN = 0x18;
      static constexpr char EM = 0x19;
      static constexpr char SUB = 0x1A;
      static constexpr char ESC = 0x1B;
      static constexpr char FS = 0x1C;
      static constexpr char GS = 0x1D;
      static constexpr char RS = 0x1E;
      static constexpr char US = 0x1F;
      static const std::map<std::string, char> toChar;
      static const std::map<char, std::string> toString;
   };

} // namespace mikado::common

#endif // CMN_ASCII_H
