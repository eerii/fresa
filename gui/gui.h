//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#ifndef DISABLE_GUI

#include "imgui.h"
#include "imgui_impl_sdl.h"

#if defined USE_OPENGL
#include "imgui_impl_opengl3.h"
#elif defined USE_VULKAN
#include "imgui_impl_vulkan.h"
#endif

#include "types.h"
#include "ecs.h"
#include "r_api.h"

//---WARNING---
//      This is a work in progress

struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };

namespace Fresa
{
    namespace Gui {
        inline ImGuiIO* io;
        inline ImGuiStyle* style;
        
        struct ActiveWindows {
            static bool test;
        };
        
        void init(Graphics::GraphicsAPI &api, const Graphics::WindowData &win);
        
        struct GuiSystem : System::PhysicsUpdate<GuiSystem, System::PRIORITY_GUI>, System::RenderUpdate<GuiSystem, System::PRIORITY_GUI> {
            static void update();
            static void render();
        };
        
        void setColors(ImVec3 text, ImVec3 head, ImVec3 area, ImVec3 body, ImVec3 pops);
    }
}

#endif
