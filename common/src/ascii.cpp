// Copyright (c) 2024 Gamaliel Ltd

#include "common/ascii.h"

using namespace std;

namespace mikado::common {
   const map<string, char> ASCII::toChar = {
      {"NUL", NUL},
      {"SOH", SOH},
      {"STX", STX},
      {"ETX", ETX},
      {"EOT", EOT},
      {"ENQ", ENQ},
      {"ACK", ACK},
      {"BEL", BEL},
      {"BS", BS},
      {"HT", HT},
      {"LF", LF},
      {"VT", VT},
      {"FF", FF},
      {"CR", CR},
      {"SO", SO},
      {"SI", SI},
      {"DLE", DLE},
      {"DC1", DC1},
      {"DC2", DC2},
      {"DC3", DC3},
      {"DC4", DC4},
      {"NAK", NAK},
      {"SYN", SYN},
      {"ETB", ETB},
      {"CAN", CAN},
      {"EM", EM},
      {"SUB", SUB},
      {"ESC", ESC},
      {"FS", FS},
      {"GS", GS},
      {"RS", RS},
      {"US", US}
   };
   const map<char, string> ASCII::toString = {
      {NUL, "NUL"},
      {SOH, "SOH"},
      {STX, "STX"},
      {ETX, "ETX"},
      {EOT, "EOT"},
      {ENQ, "ENQ"},
      {ACK, "ACK"},
      {BEL, "BEL"},
      {BS, "BS"},
      {HT, "HT"},
      {LF, "LF"},
      {VT, "VT"},
      {FF, "FF"},
      {CR, "CR"},
      {SO, "SO"},
      {SI, "SI"},
      {DLE, "DLE"},
      {DC1, "DC1"},
      {DC2, "DC2"},
      {DC3, "DC3"},
      {DC4, "DC4"},
      {NAK, "NAK"},
      {SYN, "SYN"},
      {ETB, "ETB"},
      {CAN, "CAN"},
      {EM, "EM"},
      {SUB, "SUB"},
      {ESC, "ESC"},
      {FS, "FS"},
      {GS, "GS"},
      {RS, "RS"},
      {US, "US"}
   };
}
