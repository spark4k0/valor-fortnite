#pragma once

// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <Windows.h>
#include <dwmapi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <TlHelp32.h>
#include <d3d11.h>
#include <d3dx11.h>
#include "settings.hpp"
#include "game.hpp"
#include "particles.hpp"
#include "ImGui/Assets/FontAwesome.hpp"
#include "ImGui/Assets/FontInter.hpp"
#include "ImGui/Assets/Assets.hpp"
#include "ImGui/Assets/ImagesBytes.hpp"

int CurrentTab = 0;
// Font Data
ID3D11Device* IDevice;
ID3D11ShaderResourceView* Logo;

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

float Clamp(const float& value, const float& min, const float& max) {
    return (value < min) ? min : (value > max) ? max : value;
}

static float menuAlpha = 0.0f;

bool Keybind2(CKeybind* keybind, const ImVec2& size_arg = ImVec2(0, 0), bool clicked = false, ImGuiButtonFlags flags = 0)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    // SetCursorPosX(window->Size.x - 14 - size_arg.x);
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(keybind->get_name().c_str());
    const ImVec2 label_size = ImGui::CalcTextSize(keybind->get_name().c_str(), NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) &&
        style.FramePadding.y <
        window->DC.CurrLineTextBaseOffset)  // Try to vertically align buttons that are smaller/have no padding so that
        // text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = ImGui::CalcItemSize(
        size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    bool value_changed = false;
    int key = keybind->key;

    auto io = ImGui::GetIO();

    std::string name = keybind->get_key_name();

    if (keybind->waiting_for_input)
        name = "Waiting";

    if (ImGui::GetIO().MouseClicked[0] && hovered)
    {
        if (g.ActiveId == id)
        {
            keybind->waiting_for_input = true;
        }
    }
    else if (ImGui::GetIO().MouseClicked[1] && hovered)
    {
        ImGui::OpenPopup(keybind->get_name().c_str());
    }
    else if (ImGui::GetIO().MouseClicked[0] && !hovered)
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    if (keybind->waiting_for_input)
    {
        if (ImGui::GetIO().MouseClicked[0] && !hovered)
        {
            keybind->key = VK_LBUTTON;

            ImGui::ClearActiveID();
            keybind->waiting_for_input = false;
        }
        else
        {
            if (keybind->set_key())
            {
                ImGui::ClearActiveID();
                keybind->waiting_for_input = false;
            }
        }
    }

    // Render
    ImVec4 textcolor = ImLerp(ImVec4(201 / 255.f, 204 / 255.f, 210 / 255.f, 1.f), ImVec4(1.0f, 1.0f, 1.0f, 1.f), 1.f);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(33 / 255.0f, 33 / 255.0f, 33 / 255.0f, 1.f), 2.f);
    //window->DrawList->AddRect( bb.Min, bb.Max, ImColor( 0.f, 0.f, 0.f, 1.f ) );

    window->DrawList->AddText(
        bb.Min +
        ImVec2(
            size_arg.x / 2 - ImGui::CalcTextSize(name.c_str()).x / 2,
            size_arg.y / 2 - ImGui::CalcTextSize(name.c_str()).y / 2),
        ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)),
        name.c_str());

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar;
    //ImGui::SetNextWindowPos( pos + ImVec2( 0, size_arg.y - 1 ) );
    //ImGui::SetNextWindowSize( ImVec2( size_arg.x, 47 * 1.f ) );

    {
        if (ImGui::BeginPopup(keybind->get_name().c_str(), 0))
        {

            {
                {
                    ImGui::BeginGroup();
                    {
                        if (ImGui::Selectable("hold", keybind->type == CKeybind::HOLD))
                            keybind->type = CKeybind::HOLD;
                        if (ImGui::Selectable("toggle", keybind->type == CKeybind::TOGGLE))
                            keybind->type = CKeybind::TOGGLE;
                    }
                    ImGui::EndGroup();
                }
            }

            ImGui::EndPopup();
        }
    }

    return pressed;
}
void AimbotTab()
{
    ImGui::SetCursorPos(ImVec2(20, 20));
    ImGui::CustomChild(("Aimbot"), ImVec2(ImGui::GetWindowSize().x / 2 - 30, 370));
    {
        ImGui::Checkbox(("Enabled"), &settings::aimbot::enable);
        Keybind2(&settings::aimbot::aimbotkey, ImVec2(60, 20));
        ImGui::Checkbox(("Show Fov"), &settings::aimbot::show_fov);
        /* ImGui::Checkbox(("Target NPC"), &g_Options.LegitBot.AimBot.TargetNPC);
         ImGui::Checkbox(("Visible Check"), &g_Options.LegitBot.AimBot.VisibleCheck);*/
        ImGui::Combo(("HitBox"), &settings::aimbot::aimbot_part, ("Head\0Neck\0Chest\0"));
        
        ImGui::SliderFloat(("Fov"), &settings::aimbot::fov, 50, 500);
        /*ImGui::SliderInt(("Max Distance"), &g_Options.LegitBot.AimBot.MaxDistance, 0, 600, ("%dm"));*/
        ImGui::Checkbox("Enable Smoothness", &settings::aimbot::enable_smoothness);
        ImGui::SliderFloat(("Smooth X"), &settings::aimbot::smoothness_x, 0, 30);
        ImGui::SliderFloat(("Smooth Y"), &settings::aimbot::smoothness_y, 1, 30);
        ImGui::Combo("Aimbot Type",&settings::aimbot::aimbot_type,settings::aimbot::aimbot_types,IM_ARRAYSIZE(settings::aimbot::aimbot_types),IM_ARRAYSIZE(settings::aimbot::aimbot_types));

       
    }
    ImGui::EndCustomChild();


    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 + 10, 20));
    ImGui::CustomChild(("Triggerbot"), ImVec2(ImGui::GetWindowSize().x / 2 - 30, 370));
    {
        ImGui::Checkbox(("Enabled"), &settings::triggerbot::enable_triggerbot);
        ImGui::SliderInt(("Max Distance"), &settings::triggerbot::triggerbot_delay, 1, 50);
        ImGui::SliderInt(("Reaction Time"), &settings::triggerbot::triggerbot_distance, 1, 30);
    }
    ImGui::EndCustomChild();
}

void VisualsTab()
{
    ImGui::SetCursorPos(ImVec2(20, 20));
    ImGui::CustomChild(("General"), ImVec2(ImGui::GetWindowSize().x / 2 - 30, 370));
    {
        ImGui::ColorEdit4(("Username Color"), settings::visuals::UsernameColors, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4(("Weapon Color"), settings::visuals::WeaponColors, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4(("Snaplines Color"), settings::visuals::SnaplinesColors, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4(("Rank Color"), settings::visuals::RankColors, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4(("Distance Color"), settings::visuals::DistanceColors, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4(("Platform Color"), settings::visuals::DeviceColors, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4("Skeleton Color", settings::visuals::SkeletonColors, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs);

        ImGui::Combo("Box Type", &settings::visuals::box_type, settings::visuals::box_types, IM_ARRAYSIZE(settings::visuals::box_types), IM_ARRAYSIZE(settings::visuals::box_types));
        ImGui::SliderFloat(("Skeleton Thickness"), &settings::aimbot::thickness, 1, 5);


        
    }
    ImGui::EndCustomChild();

    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 + 10, 20));
    ImGui::CustomChild(("Options"), ImVec2(ImGui::GetWindowSize().x / 2 - 30, 370));
    {
        ImGui::Checkbox(("Enabled"), &settings::visuals::enable);

        ImGui::Checkbox(("Box"), &settings::visuals::box);
        ImGui::Checkbox(("Skeleton"), &settings::visuals::skeleton);
        ImGui::Checkbox(("Username"), &settings::visuals::username);
        ImGui::Checkbox(("Platform"), &settings::visuals::platform);
        ImGui::Checkbox(("Rank"), &settings::visuals::rank);
        ImGui::Checkbox(("Weapon"), &settings::visuals::weapon);
        ImGui::Checkbox(("Distance"), &settings::visuals::distance);
        ImGui::Checkbox(("SnapLines"), &settings::visuals::line);
    }
    ImGui::EndCustomChild();
}

void MiscellaneousTab()
{
    ImGui::SetCursorPos(ImVec2(20, 20));
    ImGui::CustomChild(("Other"), ImVec2(ImGui::GetWindowSize().x / 2 - 30, 150));
    {
        ImGui::Checkbox(("Team Check"), &settings::checks::team_check);
        ImGui::Checkbox(("Visible Only"), &settings::checks::visible_check);
    }
    ImGui::EndCustomChild();

    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2 + 10, 20));
    ImGui::CustomChild(("Settings"), ImVec2(ImGui::GetWindowSize().x / 2 - 30, 140));
    {
        ImGui::Checkbox(("Streamer Mode"), &settings::aimbot::streamer);
        //ImGui::Checkbox(("SemiGod"), &g_Options.Misc.Exploits.LocalPlayer.SemiGod);
        //ImGui::Checkbox(("God"), &g_Options.Misc.Exploits.LocalPlayer.God);
        //if (ImGui::Button(("Teleport To Waypoiny")))
        //{
        //    //Cheat::Exploits::TeleportToWaypoint();
        //}
    }
    ImGui::EndCustomChild();
}
// Main code
inline std::string wstring_to_string2(const std::wstring& wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}





int render()
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEX wc;

    wc.cbClsExtra = NULL;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = TEXT("D3D11 Overlay ImGui");
    wc.lpszMenuName = nullptr;
    wc.style = CS_VREDRAW | CS_HREDRAW;

    ::RegisterClassEx(&wc);
    const std::string classNameAnsi = wstring_to_string2(wc.lpszClassName);

    const HWND hwnd = ::CreateWindowExA_Spoofed(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        classNameAnsi.c_str(),
        "D3D11 Overlay ImGui",
        WS_POPUP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );
    SetLayeredWindowAttributes_Spoofed(hwnd, 0, 255, LWA_ALPHA);
    const MARGINS margin = { -1, 0, 0, 0 };
    DwmExtendFrameIntoClientArea(hwnd, &margin);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(L"D3D11 Overlay ImGui", wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);


    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0, 0, 0, 0);

    // Main loop
    bool done = false;
    FrameWork::Assets::Initialize(IDevice);


    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (GetForegroundWindow_Spoofed() == FindWindowA_Spoofed(0, "Fortnite  ") || GetForegroundWindow_Spoofed() == hwnd)
        {
            if (settings::aimbot::enable)
            {
                settings::aimbot::aimbotkey.update();

                if (settings::aimbot::aimbotkey.enabled)
                {
                    aimbot(cache::closest_pawn);
                }
            }
        }

        game_loop();

        if (settings::aimbot::show_fov)
            ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), settings::aimbot::fov, ImColor(250, 250, 250, 250), 100, 1.0f);

        float alphaSpeed = 4.0f * ImGui::GetIO().DeltaTime;

        if (settings::overlay::show_menu)
            menuAlpha = Clamp(menuAlpha + alphaSpeed, 0.0f, 1.0f);
        else
            menuAlpha = Clamp(menuAlpha - alphaSpeed, 0.0f, 1.0f);

        if (menuAlpha > 0.0f)
        {
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImGui::GetIO().DisplaySize, ImColor(0, 0, 0, static_cast<int>(200 * menuAlpha)));

            switch (settings::overlay::particle_type)
            {   
            case 0:
                M::Particle.render(M_Particle_System::prt_type::rain);
                break;
            case 1:
                M::Particle.render(M_Particle_System::prt_type::snow);
                break;
            }

            /*ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 5.0f;
            style.FrameRounding = 3.0f;
            style.PopupRounding = 5.0f;
            style.ScrollbarRounding = 3.0f;
            style.GrabRounding = 3.0f;
            style.WindowBorderSize = 1.0f;
            style.FrameBorderSize = 1.0f;
            style.WindowPadding = ImVec2(10, 10);
            style.FramePadding = ImVec2(5, 5);*/
            ImGui::SetNextWindowSize(ImVec2(560, 410));
            ImGuiIO& io = ImGui::GetIO();

            ImGuiStyle* Style = &ImGui::GetStyle();
            Style->WindowRounding = 7;
            Style->WindowBorderSize = 1;
            Style->WindowPadding = ImVec2(0, 0);
            Style->WindowShadowSize = 0;
            Style->ScrollbarSize = 3;
            Style->ScrollbarRounding = 0;
            Style->PopupRounding = 5;


            Style->Colors[ImGuiCol_Separator] = ImColor(0, 0, 0, 0);
            Style->Colors[ImGuiCol_SeparatorActive] = ImColor(0, 0, 0, 0);
            Style->Colors[ImGuiCol_SeparatorHovered] = ImColor(0, 0, 0, 0);
            Style->Colors[ImGuiCol_ResizeGrip] = ImColor(0, 0, 0, 0);
            Style->Colors[ImGuiCol_ResizeGripActive] = ImColor(0, 0, 0, 0);
            Style->Colors[ImGuiCol_ResizeGripHovered] = ImColor(0, 0, 0, 0);
            Style->Colors[ImGuiCol_PopupBg] = ImColor(14, 14, 14);

            Style->Colors[ImGuiCol_ScrollbarBg] = ImColor(0, 0, 0, 0);
            Style->Colors[ImGuiCol_ScrollbarGrab] = ImColor(255, 0, 0);
            Style->Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(255, 0, 0);
            Style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(255, 0, 0);

            Style->Colors[ImGuiCol_WindowBg] = ImColor(14, 14, 14);
            Style->Colors[ImGuiCol_Border] = ImColor(24, 23, 25);
            
           



            ImGui::SetNextWindowBgAlpha(menuAlpha);

            ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            {
                ImDrawList* DrawList = ImGui::GetWindowDrawList();
                ImVec2 Pos = ImGui::GetWindowPos();
                ImVec2 Size = ImGui::GetWindowSize();

                DrawList->AddRectFilled(Pos, Pos + ImVec2(80, Size.y), ImColor(16, 16, 16), ImGui::GetStyle().WindowRounding, ImDrawFlags_RoundCornersLeft);
                DrawList->AddLine(Pos + ImVec2(80, 0), Pos + ImVec2(80, Size.y), ImGui::GetColorU32(ImGuiCol_Border));
                //DrawList->AddImage(Logo, Pos + ImVec2(10, 15), Pos + ImVec2(80 - 10, 80 - 10));

                //ImGui::Image(Logo, Pos + ImVec2(10, 15), Pos + ImVec2(80 - 10, 80 - 10));
                ImGui::BeginChild(("LeftChild"), ImVec2(80, Size.y));
                {
                    ImGui::SetCursorPos(ImVec2(20, 20));
                    ImGui::BeginGroup();
                    {
                        if (ImGui::Tab(("Aimbot"), ICON_FA_MOUSE, CurrentTab == 0))
                        {
                            CurrentTab = 0;
                        }
                        if (ImGui::Tab(("Visuals"), ICON_FA_EYE, CurrentTab == 1))
                        {
                            CurrentTab = 1;
                        }
                        if (ImGui::Tab(("Misc"), ICON_FA_TH, CurrentTab == 2))
                        {
                            CurrentTab = 2;
                        }
                       
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                static float AimBotChildAnim = 0;
                static float VisualsChildAnim = 0;
                static float MiscChildAnim = 0;
                static float VehiclesChildAnim = 0;
                static float PlayersChildAnim = 0;
                static float SettingsChildAnim = 0;

                AimBotChildAnim = ImLerp(AimBotChildAnim, CurrentTab == 0 ? 0.f : Size.y, ImGui::GetIO().DeltaTime * 10.f);
                VisualsChildAnim = ImLerp(VisualsChildAnim, CurrentTab == 1 ? 0.f : Size.y, ImGui::GetIO().DeltaTime * 10.f);
                MiscChildAnim = ImLerp(MiscChildAnim, CurrentTab == 2 ? 0.f : Size.y, ImGui::GetIO().DeltaTime * 10.f);
                VehiclesChildAnim = ImLerp(VehiclesChildAnim, CurrentTab == 3 ? 0.f : Size.y, ImGui::GetIO().DeltaTime * 10.f);
                PlayersChildAnim = ImLerp(PlayersChildAnim, CurrentTab == 4 ? 0.f : Size.y, ImGui::GetIO().DeltaTime * 10.f);
                SettingsChildAnim = ImLerp(SettingsChildAnim, CurrentTab == 5 ? 0.f : Size.y, ImGui::GetIO().DeltaTime * 10.f);

                ImGui::SetCursorPos(ImVec2(80, AimBotChildAnim));
                ImGui::BeginChild(("AimBotMainChild"), ImVec2(Size.x - 80, Size.y));
                {
                    AimbotTab();
                }
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(80, VisualsChildAnim));
                ImGui::BeginChild(("VisualsMainChild"), ImVec2(Size.x - 80, Size.y));
                {
                    VisualsTab();
                }
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(80, MiscChildAnim));
                ImGui::BeginChild(("MiscMainChild"), ImVec2(Size.x - 80, Size.y));
                {
                    MiscellaneousTab();
                }
                ImGui::EndChild();
            }

            ImGui::End();
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
            settings::overlay::show_menu = !settings::overlay::show_menu;

        if (settings::overlay::show_menu)
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        else
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(L"D3D11 Overlay ImGui", wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}