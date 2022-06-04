//: fresa by jose pazos perez, licensed under GPLv3
#ifndef DISABLE_GUI

#include "gui.h"
#include "scene.h"
#include "reflection.h"

using namespace Fresa;

void Gui::win_entities() {
    if (ImGui::Begin("entities")) {
        if (not scene_list.count(active_scene)) {
            ImGui::Text("no active scene");
        } else {
            Scene &scene = scene_list.at(active_scene);
            
            static ImGuiTableFlags flags = ImGuiTableFlags_PadOuterX | ImGuiTableFlags_RowBg;
            if (ImGui::BeginTable("inspector", 2, flags))
            {
                ImGui::TableSetupColumn("entity", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("property", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                
                static ui32 n = 0;
                for (EntityID e : SceneView<>(scene)) {
                    ImGui::PushID(e);
                    ImGui::TableNextRow(); n++;
                    
                    //: Entity
                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    bool entity_open = ImGui::TreeNode(scene.getName(e).c_str());
                    
                    //: Properties
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("id: [%d]", Entity::getIndex(e));
                    
                    //: Submenu
                    if (entity_open) {
                        Signature mask = scene.getMask(e);
                        
                        for_<Component::ComponentType>([&](auto i){
                            if (mask[i.value] != 0) {
                                using C = std::variant_alternative_t<i.value, Component::ComponentType>;
                                    
                                ImGui::PushID(i.value);
                                ImGui::TableNextRow();
                                
                                //: Component name
                                ImGui::TableSetColumnIndex(0);
                                ImGui::AlignTextToFramePadding();
                                bool component_open = ImGui::TreeNode(lower(str(type_name<C>())).c_str());
                                
                                //: Component members
                                if (component_open) {
                                    C* component = scene.getComponent<C>(e);
                                    
                                    for_<Reflection::as_type_list<C>>([&](auto i){
                                        using M = std::variant_alternative_t<i.value, Reflection::as_type_list<C>>;
                                        auto x = Reflection::get_member_i<i.value>(component);
                                        
                                        ImGui::PushID(i.value);
                                        ImGui::TableNextRow();
                                        
                                        //: Member name
                                        ImGui::TableSetColumnIndex(0);
                                        ImGui::AlignTextToFramePadding();
                                        ImGui::Text("%s", C::member_names.at(i.value));
                                        
                                        //: Member value
                                        ImGui::TableSetColumnIndex(1);
                                        Gui::value(*x);
                                        
                                        ImGui::PopID();
                                    });
                                    
                                    ImGui::TreePop();
                                }
                                
                                ImGui::PopID();
                            }
                        });
                        
                        ImGui::TreePop();
                    }
                    
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        
        ImGui::End();
    }
}

#endif
