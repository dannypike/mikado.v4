#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(BRK_GUI_H)
#define BRK_GUI_H

#include "imgui.h"

namespace mikado::broker {

   class Gui : public mikado::gui::GuiBase {
      typedef common::MikadoErrorCode MikadoErrorCode;

   public:
      Gui(common::ConfigurePtr cfg);
      ~Gui();

   private:
      common::ConfigurePtr cfg_;
   };
   typedef std::shared_ptr<Gui> GuiPtr;

} // namespace mikado::broker

#endif // BRK_GUI_H
