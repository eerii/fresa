//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "gui_entities.h"
#include "gui_types.h"
#include "r_textures.h"
#include "r_window.h"

#include "log.h"
#include "system_list.h"
#include "serialization.h"

using namespace Verse;

namespace {
    ui32 n = 0;
    std::map<str, std::function<void(Config&, EntityID)>> c_funcs;
}

void components(Config &c, Signature mask, EntityID e);

void Gui::entities(Config &c) {
    ImGui::Begin("entities");
    
    ImGui::Text("scene: %s", ((c.active_scene != nullptr) ? c.active_scene->name.c_str() : "NULL"));
    ImGui::Text("number of entities: %d", n);
    
    if (ImGui::SmallButton("save (temp)"))
        Serialization::saveScene(c.active_scene, c);
    ImGui::SameLine();
    if (ImGui::SmallButton("save (project)"))
        Serialization::saveScene(c.active_scene, c, true);
    
    static ImGuiTableFlags flags = ImGuiTableFlags_PadOuterX | ImGuiTableFlags_RowBg;
    
    if (ImGui::BeginTable("inspector", 2, flags))
    {
        ImGui::TableSetupColumn("entity", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("property", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        
        n = 0;
        for (EntityID e : SceneView<>(*c.active_scene))
        {
            ImGui::PushID(e);
            Entity::EntityIndex index = Entity::getIndex(e);
            n++;
            
            ImGui::TableNextRow();
            
            //Entity Name
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            str entity_name = c.active_scene->entity_names[index];
            bool node_open = ImGui::TreeNode(entity_name.c_str());
            
            //Properties
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("id: [%d]", index);
            
            ImGui::SameLine();
            str add_label = "add##" + std::to_string(e);
            if (ImGui::SmallButton(add_label.c_str())) {
                
            }
            
            ImGui::SameLine();
            str remove_label = "remove##" + std::to_string(e);
            if (ImGui::SmallButton(remove_label.c_str())) {
                c.active_scene->removeEntity(e);
            }
            
            //Submenu
            if (node_open) {
                Signature mask = c.active_scene->mask[index];
                
                components(c, mask, e);
                
                ImGui::TreePop();
            }
            
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    
    ImGui::End();
}

void components(Config &c, Signature mask, EntityID e) {
    if (c_funcs.size() == 0) {
        c_funcs["collider"] = System::Collider::gui;
        c_funcs["actor"] = System::Actor::gui;
        c_funcs["state"] = System::State::gui;
        c_funcs["texture"] = System::Texture::gui;
        c_funcs["animation"] = System::Animation::gui;
        c_funcs["tilemap"] = System::Tilemap::gui;
        c_funcs["camera"] = System::Camera::gui;
        c_funcs["light"] = System::Light::gui;
        c_funcs["timer"] = System::Timer::gui;
        c_funcs["patrol"] = System::Patrol::gui;
        c_funcs["sceneTransition"] = System::SceneTransition::gui;
#ifdef USE_C_FIRE
        c_funcs["fire"] = System::Fire::gui;
#endif
#ifdef USE_C_PLAYER
        c_funcs["player"] = System::Player::gui;
#endif
    }
    
    for (int i = 0; i < MAX_COMPONENTS; i++) {
        if (mask[i] == 0)
            continue;
        
        ImGui::PushID(i);
        ImGui::TableNextRow();
        
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        str component_name = Component::getName(i);
        component_name[0] = std::tolower(component_name[0]);
        bool node_open = ImGui::TreeNode(component_name.c_str());
        
        ImGui::TableSetColumnIndex(1);
        str remove_component_label = "remove##" + std::to_string(e) + std::to_string(i);
        if (ImGui::SmallButton(remove_component_label.c_str())) {
            c.active_scene->removeComponent(e, i);
        }
        
        if (node_open) {
            ImGui::TableNextRow();
            
            c_funcs[component_name](c, e);
            
            ImGui::TreePop();
        }
        
        ImGui::PopID();
    }
}
