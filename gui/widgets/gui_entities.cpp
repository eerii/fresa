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
#include "input.h"

using namespace Verse;

namespace {
    ui32 n = 0;
    str add_entity = "";
    std::map<str, std::function<void(Config&, EntityID)>> c_funcs;
}

void components(Config &c, Signature mask, EntityID e);

void Gui::entities(Config &c) {
    ImGui::Begin("entities");
    
    ImGui::AlignTextToFramePadding();
    ImGui::Text("scene: %s", ((c.active_scene != nullptr) ? c.active_scene->name.c_str() : "NULL"));
    ImGui::SameLine();
    if (Input::shift()) {
        if (ImGui::SmallButton("save in project"))
            Serialization::saveScene(c.active_scene, c, true);
    } else {
        if (ImGui::SmallButton("save"))
            Serialization::saveScene(c.active_scene, c);
    }
    
    ImGui::AlignTextToFramePadding();
    ImGui::Text("number of entities: %d", n);
    
    ImGui::AlignTextToFramePadding();
    ImGui::Text("add entity:");
    ImGui::SameLine();
    if (ImGui::SmallButton("+") and add_entity.size() > 0) {
        c.active_scene->createEntity(add_entity);
        add_entity = "";
    }
    ImGui::SameLine();
    str input_label = "##addentity";
    ImGui::InputText(input_label.c_str(), &add_entity);
    
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
            str remove_label = "remove##entity" + std::to_string(e);
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
        c_funcs["circle_collider"] = System::Collider::gui_circle;
        c_funcs["actor"] = System::Actor::gui;
        c_funcs["state"] = System::State::gui;
        c_funcs["texture"] = System::Texture::gui;
        c_funcs["animation"] = System::Animation::gui;
        c_funcs["tilemap"] = System::Tilemap::gui;
        c_funcs["text"] = System::Text::gui;
        c_funcs["camera"] = System::Camera::gui;
        c_funcs["light"] = System::Light::gui;
        c_funcs["noise"] = System::Noise::gui;
        c_funcs["timer"] = System::Timer::gui;
        c_funcs["patrol"] = System::Patrol::gui;
        c_funcs["scene_transition"] = System::SceneTransition::gui;
        c_funcs["player"] = System::Player::gui;
    }
    
    std::vector<str> unused_components = {" - none - "};
    
    for (int i = 0; i < component_names.size(); i++) {
        str component_name = component_names[i];
        
        if (mask[i] == 0) {
            unused_components.push_back(component_name);
            continue;
        }
        
        ImGui::PushID(i);
        ImGui::TableNextRow();
        
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        bool node_open = ImGui::TreeNode(component_name.c_str());
        
        ImGui::TableSetColumnIndex(1);
        str remove_component_label = "remove##component" + std::to_string(e) + std::to_string(i);
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
    
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("add component");
    
    ImGui::TableSetColumnIndex(1);
    str add_label = "##addcomponent" + std::to_string(e);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    int curr = 0;
    if (ImGui::Combo(add_label.c_str(), &curr, unused_components) and curr != 0) {
        auto it = std::find(component_names.begin(), component_names.end(), unused_components[curr]);
        if (it != component_names.end()) {
            ComponentID cid = std::distance(component_names.begin(), it);
            //TODO: FIX GUI COMPONENTS
            /*for_<std::variant_size_v<ComponentType>>([&](auto i) {
                if (i.value == cid)
                    c.active_scene->addComponent<std::variant_alternative_t<i.value, ComponentType>>(e);
            });*/
        }
    }
}
