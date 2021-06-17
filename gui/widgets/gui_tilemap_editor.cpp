//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "gui_tilemap_editor.h"
#include "gui_types.h"

#include "s_tilemap.h"
#include "log.h"

using namespace Verse;

namespace {
    Vec2 size;
}

void Gui::tilemapEditor(Config &c) {
    ImGui::Begin("tilemap editor");
    
    static ImGuiTableFlags flags = ImGuiTableFlags_PadOuterX | ImGuiTableFlags_RowBg;
    
    if (ImGui::BeginTable("tmap editor", 2, flags))
    {
        ImGui::TableSetupColumn("prop", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
        
        ImGui::TableNextRow();
        
        
        Verse::Gui::draw_vec2(c.tme_curr_tmap->pos.x, c.tme_curr_tmap->pos.y, "pos", c.tme_curr_id, [&c]() {
            Component::Collider* col = c.active_scene->getComponent<Component::Collider>(c.tme_curr_id);
            if (col->transform.pos() != c.tme_curr_tmap->pos) {
                col->transform = c.tme_curr_tmap->pos;
                System::Tilemap::createVertices(c, c.tme_curr_tmap);
            }
        });
        ImGui::TableNextRow();
        
        
        Verse::Gui::draw_vec2(size.x, size.y, "size", c.tme_curr_id, [&c]() {
            Vec2 prev_size = Vec2((int)c.tme_curr_tmap->tiles[0].size(), (int)c.tme_curr_tmap->tiles.size() - 1);
            
            //Delete X
            while (size.x < prev_size.x) {
                if (prev_size.x <= 1)
                    break;
                
                for (int i = 0; i < c.tme_curr_tmap->tiles.size() - 1; i++) {
                    c.tme_curr_tmap->tiles[i].pop_back();
                }
                prev_size.x--;
            }
            //Add X
            while (size.x > prev_size.x) {
                for (int i = 0; i < c.tme_curr_tmap->tiles.size() - 1; i++) {
                    c.tme_curr_tmap->tiles[i].push_back(255);
                }
                prev_size.x++;
            }
            //Delete Y
            while (size.y < prev_size.y + 1) {
                if (prev_size.y <= 1)
                    break;
                
                c.tme_curr_tmap->tiles.pop_back();
                prev_size.y--;
            }
            //Add Y
            while (size.y > prev_size.y) {
                c.tme_curr_tmap->tiles.push_back(std::vector<ui8>(c.tme_curr_tmap->tiles[0].size(), 255));
                prev_size.y++;
            }
            
            Component::Collider* col = c.active_scene->getComponent<Component::Collider>(c.tme_curr_id);
            col->transform.w = size.x * c.tme_curr_tmap->tex_size.x;
            col->transform.h = size.y * c.tme_curr_tmap->tex_size.y;
            
            System::Tilemap::createVertices(c, c.tme_curr_tmap);
        });
        size = Vec2((int)c.tme_curr_tmap->tiles[0].size(), (int)c.tme_curr_tmap->tiles.size() - 1);
        ImGui::TableNextRow();
        
        
        Verse::Gui::draw_vec2(c.tme_curr_tmap->tex_size.x, c.tme_curr_tmap->tex_size.y, "tex size", c.tme_curr_id, [&c]() {
            System::Tilemap::createVertices(c, c.tme_curr_tmap);
        });
        ImGui::TableNextRow();
        
        ImGui::EndTable();
    }
    
    ImGui::End();
}
