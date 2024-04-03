// Copyright (c) 2024 Gamaliel Ltd
#include "common.h"
#include "windowsApi.h"
#include "imgui.h"

using namespace std;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace mikado::gui {

   LPDIRECT3D9 GuiBase::direct3D_ = nullptr;
   LPDIRECT3DDEVICE9 GuiBase::d3dDevice_ = nullptr;
   UINT GuiBase::resizeWidth_ = 0;
   UINT GuiBase::resizeHeight_ = 0;
   D3DPRESENT_PARAMETERS GuiBase::d3dParams_ = {};

   WNDCLASSEXW GuiBase::wc_ = {
      sizeof(wc_), CS_CLASSDC, GuiBase::WndProc, 0L, 0L
      , GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"MikadoWindowClass", nullptr
   };

   GuiBase::GuiBase(string const &windowTitle)
         : clearColor_(0.45f, 0.55f, 0.60f, 1.00f)
         , windowTitle_(windowTitle) {

      str_debug() << "creating gui window" << endl;
      createWindow();
   }

   GuiBase::~GuiBase() {
      destroyWindow();
   }

   bool GuiBase::createWindow() {
      // Create application window
      ImGui_ImplWin32_EnableDpiAwareness();

      ::RegisterClassExW(&wc_);
      hWnd_ = ::CreateWindowW(wc_.lpszClassName, common::toString(windowTitle_).c_str()
         , WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc_.hInstance, nullptr);
      if (!hWnd_) {
         return false;
      }

      // Initialize Direct3D
      if (!CreateDeviceD3D(hWnd_)) {
         CleanupDeviceD3D();
         ::UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
         return false;
      }

      // Show the window
      ::ShowWindow(hWnd_, SW_SHOWDEFAULT);
      ::UpdateWindow(hWnd_);

      createImguiContext();
      createFonts();
      return true;
   }

   void GuiBase::destroyWindow() {
      ImGui_ImplDX9_Shutdown();
      ImGui_ImplWin32_Shutdown();
      ImGui::DestroyContext();

      CleanupDeviceD3D();

      if (hWnd_) {
         ::DestroyWindow(hWnd_);
         ::UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
         hWnd_ = nullptr;
      }
   }

   void GuiBase::createImguiContext(bool darkStyle) {
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO &io = ImGui::GetIO(); (void)io;
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
      io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
      //io.ConfigViewportsNoAutoMerge = true;
      //io.ConfigViewportsNoTaskBarIcon = true;

      // Setup Dear ImGui style
      if (darkStyle) {
         ImGui::StyleColorsDark();
      }
      else {
         ImGui::StyleColorsLight();
      }

      // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
      ImGuiStyle &style = ImGui::GetStyle();
      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
         style.WindowRounding = 0.0f;
         style.Colors[ImGuiCol_WindowBg].w = 1.0f;
      }

      // Setup Platform/Renderer backends
      ImGui_ImplWin32_Init(hWnd_);
      ImGui_ImplDX9_Init(d3dDevice_);
   }

   bool GuiBase::CreateDeviceD3D(HWND hWnd) {
      if ((direct3D_ = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
         return false;

      // Create the D3DDevice
      ZeroMemory(&d3dParams_, sizeof(d3dParams_));
      d3dParams_.Windowed = TRUE;
      d3dParams_.SwapEffect = D3DSWAPEFFECT_DISCARD;
      d3dParams_.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
      d3dParams_.EnableAutoDepthStencil = TRUE;
      d3dParams_.AutoDepthStencilFormat = D3DFMT_D16;
      d3dParams_.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
      //d3dParams_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
      if (direct3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dParams_, &d3dDevice_) < 0)
         return false;

      return true;
   }

   void GuiBase::ResetDevice() {
      ImGui_ImplDX9_InvalidateDeviceObjects();
      HRESULT hr = d3dDevice_->Reset(&d3dParams_);
      if (hr == D3DERR_INVALIDCALL)
         IM_ASSERT(0);
      ImGui_ImplDX9_CreateDeviceObjects();
   }

   void GuiBase::CleanupDeviceD3D() {
      if (d3dDevice_) {
         d3dDevice_->Release();
         d3dDevice_ = nullptr;
      }
      if (direct3D_) {
         direct3D_->Release();
         direct3D_ = nullptr;
      }
   }

   void GuiBase::createFonts() {
      // Load Fonts
      // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
      // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
      // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
      // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
      // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
      // - Read 'docs/FONTS.md' for more instructions and details.
      // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
      //io.Fonts->AddFontDefault();
      //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
      //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
      //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
      //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
      //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
      //IM_ASSERT(font != nullptr);
   }

   LRESULT WINAPI GuiBase::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
      if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
         return true;

      switch (msg) {
      case WM_SIZE:
         if (wParam == SIZE_MINIMIZED)
            return 0;
         resizeWidth_ = (UINT)LOWORD(lParam); // Queue resize to be processed outside the PeekMessage() loop
         resizeHeight_ = (UINT)HIWORD(lParam);
         return 0;

      case WM_SYSCOMMAND:
         if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
         break;

      case WM_DESTROY:
         ::PostQuitMessage(0);
         return 0;

      case WM_DPICHANGED:
         if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports) {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT *suggested_rect = (RECT *)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
         }
         break;
      }
      return ::DefWindowProcW(hWnd, msg, wParam, lParam);
   }

   bool GuiBase::processWin32Messages(bool &workDone) {
      MSG msg;

      while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
         workDone = true;
         ::TranslateMessage(&msg);
         ::DispatchMessage(&msg);
         if (WM_QUIT == msg.message) {
            return false;
         }
      }

      // Handle window resize (we don't resize directly in the WM_SIZE handler)
      if ((0 != resizeWidth_) && (0 != resizeHeight_)) {
         workDone = true;
         d3dParams_.BackBufferWidth = resizeWidth_;
         d3dParams_.BackBufferHeight = resizeHeight_;
         resizeWidth_ = resizeWidth_ = 0;
         ResetDevice();
      }

      return true;
   }

} // namespace mikado::gui
