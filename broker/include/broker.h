#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(BROKER_H)
#define BROKER_H

namespace mikado::broker {

   unsigned int const BROKER_VERSION_MAJOR = 0;
   unsigned int const BROKER_VERSION_MINOR = 1;
    
   void outputBanner();
   common::MikadoErrorCode main(int argc, char *argv[]);

} // namespace mikado::broker

#include "broker/appId.h"
#include "broker/appInstanceId.h"
#include "broker/app.h"
#include "broker/appManager.h"
#include "broker/gui.h"
#include "broker/handler.h"

#endif // BROKER_H
