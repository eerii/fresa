//: fresa by jose pazos perez, licensed under GPLv3
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
        //---ImGui parameters---
        inline ImGuiIO* io;
        inline ImGuiStyle* style;
        
        //---Gui functions---
        void init();
        
        struct GuiSystem : System::PhysicsUpdate<GuiSystem, System::PRIORITY_GUI>, System::RenderUpdate<GuiSystem, System::PRIORITY_GUI> {
            static void update();
            static void render();
        };
        
        void setColors(ImVec3 text, ImVec3 head, ImVec3 area, ImVec3 body, ImVec3 pops);
        
        //---Windows---
        
        //: List
        struct Window {
            Window(str n, std::function<void()> f, bool a = false) : name(n), show(f), active(a) {};
            str name;
            std::function<void()> show;
            bool active;
        };
        inline std::vector<Window> windows;
        
        //: Functions
        void win_menu();
        void win_performance();
        void win_entities();
        
        //: Register
        inline void registerWindows() {
            windows.push_back(Window("menu", win_menu, true));
            windows.push_back(Window("entities", win_entities));
            windows.push_back(Window("performance", win_performance));
            windows.push_back(Window("test", [](){ImGui::ShowDemoWindow();}));
        }
        
        //---Widgets---
        inline ui64 widget_id = 0;
        inline str get_name_id(str name) {return name + "##" + std::to_string(widget_id++);}
        
        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        bool slider(str name, T& value, float step, T min, T max, str fmt = "", bool log = false) {
            ImGuiDataType type = ImGuiDataType_COUNT;
            if (std::is_same_v<T, ui8>) type = ImGuiDataType_U8;
            if (std::is_same_v<T, ui16>) type = ImGuiDataType_U16;
            if (std::is_same_v<T, ui32>) type = ImGuiDataType_U32;
            if (std::is_same_v<T, ui64>) type = ImGuiDataType_U64;
            if (std::is_same_v<T, int>) type = ImGuiDataType_S32;
            if (std::is_same_v<T, long>) type = ImGuiDataType_S64;
            if (std::is_same_v<T, float>) type = ImGuiDataType_Float;
            if (std::is_same_v<T, double>) type = ImGuiDataType_Double;
            if (type == ImGuiDataType_COUNT)
                log::error("Slider type not allowed");
            
            if (fmt == "")
                return ImGui::DragScalar(get_name_id(name).c_str(), type, &value, step, &min, &max);
            
            if (log == false)
                return ImGui::DragScalar(get_name_id(name).c_str(), type, &value, step, &min, &max, fmt.c_str());
            
            return ImGui::DragScalar(get_name_id(name).c_str(), type, &value, step, &min, &max, fmt.c_str(), ImGuiSliderFlags_Logarithmic);
        }
        
        template<typename T>
        bool slider(str name, T& value) {
            Vec2<T> clamp{};
            if (std::is_same_v<T, ui8>) clamp = Vec2<T>(0, 255);
            if (std::is_same_v<T, ui16>) clamp = Vec2<T>(0, 1000);
            if (std::is_same_v<T, ui32>) clamp = Vec2<T>(0, 1000);
            if (std::is_same_v<T, ui64>) clamp = Vec2<T>(0, 1000);
            if (std::is_same_v<T, int>) clamp = Vec2<T>(-1000, 1000);
            if (std::is_same_v<T, long>) clamp = Vec2<T>(-1000, 1000);
            if (std::is_same_v<T, float>) clamp = Vec2<T>(-1000.0f, 1000.0f);
            if (std::is_same_v<T, double>) clamp = Vec2<T>(-1000.0, 1000.0);
            
            float step = 0.1f;
            if (std::is_integral_v<T>)
                step = 1.0f;
            
            return slider(name, value, step, clamp.x, clamp.y);
        }
        
        template<typename T>
        void value(T &x, std::function<void()> callback = [](){return;}) {
            //: Strings
            if constexpr (std::is_same_v<T, str>)
                ImGui::Text("%s", x.c_str());
            
            //: Bool
            if constexpr (std::is_same_v<T, bool>)
                if (ImGui::Checkbox(get_name_id("").c_str(), &x)) callback();
            
            //: Number
            if constexpr (std::is_arithmetic_v<T> and not std::is_same_v<T, bool>)
                if (slider("", x)) callback();
            
            //: Vec2
            if constexpr (is_vec2_v<T>) {
                float line_height = style->FramePadding.y * 2.0f + ImGui::CalcTextSize("X").y;
                ImVec2 button_size = { line_height + 3.0f, line_height };
                
                //: X
                if (ImGui::Button("X", button_size)) x.x = decltype(x.x){};

                //: X value
                ImGui::SameLine();
                ImGui::SetNextItemWidth((ImGui::GetColumnWidth() - button_size.x) * 0.5f - (2.0f * style->FramePadding.x));
                if (slider("", x.x)) callback();
                
                //: Y
                ImGui::SameLine();
                if (ImGui::Button("Y", button_size)) x.y = decltype(x.y){};

                //: Y value
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
                if (slider("", x.y)) callback();
            }
            
            //: Rect2
            if constexpr (is_rect2_v<T>) {
                float line_height = style->FramePadding.y * 2.0f + ImGui::CalcTextSize("X").y;
                ImVec2 button_size = { line_height + 3.0f, line_height };
                
                //: X
                if (ImGui::Button("X", button_size)) x.x = decltype(x.x){};

                //: X value
                ImGui::SameLine();
                ImGui::SetNextItemWidth((ImGui::GetColumnWidth() - button_size.x) * 0.5f - (2.0f * style->FramePadding.x));
                if (slider("", x.x)) callback();
                
                //: Y
                ImGui::SameLine();
                if (ImGui::Button("Y", button_size)) x.y = decltype(x.y){};

                //: Y value
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
                if (slider("", x.y)) callback();
                
                //- Line break
                
                //: W
                if (ImGui::Button("W", button_size)) x.w = decltype(x.w){};

                //: W value
                ImGui::SameLine();
                ImGui::SetNextItemWidth((ImGui::GetColumnWidth() - button_size.x) * 0.5f - (2.0f * style->FramePadding.x));
                if (slider("", x.w)) callback();
                
                //: H
                ImGui::SameLine();
                if (ImGui::Button("H", button_size)) x.h = decltype(x.h){};

                //: H value
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
                if (slider("", x.h)) callback();
            }
            
            //: std::vector
            if constexpr (is_vector_v<T>) {
                for (auto &v : x)
                    value(v);
            }
        }
    }
}

#endif
