//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "gui.h"
#include <functional>

#ifndef DISABLE_GUI

namespace Verse::Gui
{

[[maybe_unused]]
static void draw_vec2(int &x, int &y, str label, EntityID eid = 0,
                      std::function<void()> callback = []() {return;}, float reset = 0.0f) {
    ImGuiStyle& style = ImGui::GetStyle();
    
    ImGui::PushID(label.c_str());
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", label.c_str());
    
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

[[maybe_unused]]
static void draw_vec2(float &x, float &y, str label, EntityID eid = 0,
                      std::function<void()> callback = []() {return;}, float reset = 0.0f) {
    ImGuiStyle& style = ImGui::GetStyle();
    
    ImGui::PushID(label.c_str());
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", label.c_str());
    
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

[[maybe_unused]]
static void draw_int(int &i, str label, EntityID eid = 0, std::function<void()> callback = []() {return;}) {
    ImGui::PushID(label.c_str());
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", label.c_str());
    
    ImGui::TableSetColumnIndex(1);
    str i_label = "##" + label + std::to_string(eid);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    if (ImGui::DragInt(i_label.c_str(), &i))
        callback();
    
    ImGui::PopID();
}

[[maybe_unused]]
static void draw_ui8(ui8 &i, str label, EntityID eid = 0, std::function<void()> callback = []() {return;}) {
    ImGui::PushID(label.c_str());
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", label.c_str());
    
    ImGui::TableSetColumnIndex(1);
    str i_label = "##" + label + std::to_string(eid);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    if (ImGui::DragInt(i_label.c_str(), reinterpret_cast<int*>(&i), 1, 0, 255))
        callback();
    
    ImGui::PopID();
}

[[maybe_unused]]
static void draw_float(float &f, str label, EntityID eid = 0, std::function<void()> callback = []() {return;}) {
    ImGui::PushID(label.c_str());
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", label.c_str());
    
    ImGui::TableSetColumnIndex(1);
    str f_label = "##" + label + std::to_string(eid);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    if (ImGui::DragFloat(f_label.c_str(), &f, 0.1f, -100000, 100000, "%.1f"))
        callback();
    
    ImGui::PopID();
}

[[maybe_unused]]
static void draw_bool(bool &b, str label, EntityID eid = 0, std::function<void()> callback = []() {return;}) {
    ImGui::PushID(label.c_str());
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", label.c_str());
    
    ImGui::TableSetColumnIndex(1);
    str i_label = "##" + label + std::to_string(eid);
    if (ImGui::Checkbox(i_label.c_str(), &b))
        callback();
    
    ImGui::PopID();
}

}

#endif
