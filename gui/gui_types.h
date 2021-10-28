//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "gui.h"
#include <functional>

#ifndef DISABLE_GUI

namespace Verse::Gui
{

inline void draw_label(str label, EntityID eid = 0) {
    ImGui::PushID(label.c_str());
    
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label.c_str());
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
void draw_int(T &n, str label, EntityID eid = 0, std::function<void()> callback = [](){},
                                                 float speed = 1.0f, int min_val = -1e3, int max_val = 1e3) {
    draw_label(label, eid);
    
    ImGui::TableSetColumnIndex(1);
    str num_label = "##" + label + std::to_string(eid);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    
    bool is_unsigned = !std::is_signed_v<T>; //TODO: Make constexpr with https://github.com/bolero-MURAKAMI/Sprout
    int size = std::log2(sizeof(T));
    int index = 2 * size + (int)is_unsigned;
    
    if (ImGui::DragScalar(num_label.c_str(), index, &n, speed, &min_val, &max_val))
        callback();
    
    ImGui::PopID();
}

template<typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
void draw_float(T &f, str label, EntityID eid = 0, std::function<void()> callback = []() {},
                                                         float speed = 0.1f, float min_val = -1.0e3, float max_val = 1.0e3) {
    draw_label(label, eid);
    
    ImGui::TableSetColumnIndex(1);
    str num_label = "##" + label + std::to_string(eid);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    
    int index = (sizeof(T) == 4) ? 8 : 9;
    
    if (ImGui::DragScalar(num_label.c_str(), index, &f, speed, &min_val, &max_val, "%.1f"))
        callback();
    
    ImGui::PopID();
}

inline void draw_bool(bool &b, str label, EntityID eid = 0, std::function<void()> callback = []() {return;}) {
    draw_label(label, eid);
    
    ImGui::TableSetColumnIndex(1);
    str i_label = "##" + label + std::to_string(eid);
    if (ImGui::Checkbox(i_label.c_str(), &b))
        callback();
    
    ImGui::PopID();
}

//TODO: Rework draw vec2 and add draw for other types
inline void draw_vec2(int &x, int &y, str label, EntityID eid = 0,
                      std::function<void()> callback = []() {return;}, float reset = 0.0f) {
    draw_label(label, eid);
    ImGuiStyle& style = ImGui::GetStyle();
    
    float line_height = style.FramePadding.y * 2.0f + ImGui::CalcTextSize("X").y;
    ImVec2 button_size = { line_height + 3.0f, line_height };
    
    ImGui::TableSetColumnIndex(1);
    
    if (ImGui::Button("X", button_size))
        x = reset;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth() * 0.5f - button_size.x);
    str x_label = "##X" + std::to_string(eid);
    if (ImGui::DragInt(x_label.c_str(), &x))
        callback();
    ImGui::SameLine();

    if (ImGui::Button("Y", button_size))
        y = reset;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    str y_label = "##Y" + std::to_string(eid);
    if (ImGui::DragInt(y_label.c_str(), &y))
        callback();
    ImGui::SameLine();
    
    ImGui::PopID();
}

inline void draw_vec2(float &x, float &y, str label, EntityID eid = 0,
                      std::function<void()> callback = []() {return;}, float reset = 0.0f) {
    draw_label(label, eid);
    ImGuiStyle& style = ImGui::GetStyle();
    
    float line_height = style.FramePadding.y * 2.0f + ImGui::CalcTextSize("X").y;
    ImVec2 button_size = { line_height + 3.0f, line_height };
    
    ImGui::TableSetColumnIndex(1);
    
    if (ImGui::Button("X", button_size))
        x = reset;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth() * 0.5f - button_size.x);
    str x_label = "##X" + std::to_string(eid);
    if (ImGui::DragFloat(x_label.c_str(), &x, 0.1f, -100000, 100000, "%.1f"))
        callback();
    ImGui::SameLine();

    if (ImGui::Button("Y", button_size))
        y = reset;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    str y_label = "##Y" + std::to_string(eid);
    if (ImGui::DragFloat(y_label.c_str(), &y, 0.1f, -100000, 100000, "%.1f"))
        callback();
    ImGui::SameLine();
    
    ImGui::PopID();
}

}

#endif
