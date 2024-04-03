// Copyright (c) 2024 Gamaliel Ltd

#include "common.h"
#include "imgui.h"
#include "windowsApi.h"
#include "broker/gui.h"

using namespace std;

namespace mikado::broker {
   typedef common::MikadoErrorCode MikadoErrorCode;

   ///////////////////////////////////////////////////////////////////////////
   //
   Gui::Gui(common::ConfigurePtr cfg)
      : GuiBase(common::kWindowBroker)
      , cfg_(cfg){
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   Gui::~Gui() {
   }

} // namespace mikado::broker
