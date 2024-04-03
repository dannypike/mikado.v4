#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(MKO_GUI_BASE_H)
#define MKO_GUI_BASE_H

#include "imgui/guiBase.h"

namespace mikado::gui {

   class GuiBase : public std::enable_shared_from_this<GuiBase> {
   public:
      GuiBase(std::string const &windowTitle);
      virtual ~GuiBase();

      virtual void render() = 0;

   protected:
      bool createWindow();
      void destroyWindow();
      void createImguiContext(bool darkStyle = true);
      static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
      bool CreateDeviceD3D(HWND hWnd);
      void ResetDevice();
      void CleanupDeviceD3D();
      void createFonts();
      bool processWin32Messages(bool &workDone);

   private:
      static LPDIRECT3D9 direct3D_;
      static LPDIRECT3DDEVICE9 d3dDevice_;
      static UINT resizeWidth_, resizeHeight_;
      static D3DPRESENT_PARAMETERS d3dParams_;
      static WNDCLASSEXW wc_;

      HWND hWnd_;
      ImVec4 clearColor_;
      std::string windowTitle_;
   };

} // namespace mikado::gui

#endif // MKO_GUI_BASE_H     
