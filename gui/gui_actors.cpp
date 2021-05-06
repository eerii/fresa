//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include <math.h>

#include "gui_actors.h"
#include "imgui.h"

using namespace Verse;

void Gui::actors(Scene &scene) {
#ifdef ACTOR
#ifdef COLLIDER
    ImGui::Begin("Actors");
    
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_RowBg;
    
    if (ImGui::BeginTable("Position", 4, flags))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed, 96.0f);
        ImGui::TableSetupColumn("Velocity", ImGuiTableColumnFlags_WidthFixed, 128.0f);
        ImGui::TableSetupColumn("On Ground", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        
        for (EntityID ent : SceneView<Component::Actor>(scene))
        {
            Entity::EntityIndex index = Entity::getIndex(ent);
            
            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", scene.entity_names[index].c_str());
            
            ImGui::TableNextColumn();
            ImGui::Text("x: %d, y:%d",
                        (int)scene.getComponent<Component::Collider>(ent)->transform.x,
                        (int)scene.getComponent<Component::Collider>(ent)->transform.y);
            
            ImGui::TableNextColumn();
            ImGui::Text("x: %.2f, y:%.2f",
                        scene.getComponent<Component::Actor>(ent)->vel.x,
                        scene.getComponent<Component::Actor>(ent)->vel.y);
            
            ImGui::TableNextColumn();
            ImGui::Text("%s", scene.getComponent<Component::Actor>(ent)->is_on_ground ? "true" : "false");
        }
        ImGui::EndTable();
    }
    
    ImGui::End();
#endif
#endif
}
