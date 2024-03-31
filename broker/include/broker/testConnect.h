#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_TEST_CONNECT_H)
#define CMN_TEST_CONNECT_H

namespace mikado::broker {

void testConnect(common::ConfigurePtr options);  // simulate a connection from another application
bool testProcess();  // idle method to simulate processing
void testShutdown();

} // namespace mikado::common

#endif // CMN_TEST_CONNECT_H
