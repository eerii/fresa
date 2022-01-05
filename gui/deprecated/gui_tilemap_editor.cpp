//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#ifndef DISABLE_GUI

/*
#include "gui_tilemap_editor.h"
#include "gui_types.h"

#include "s_tilemap.h"
#include "log.h"

using namespace Fresa;

namespace {
    Vec2 size;
}

void Gui::tilemapEditor(Config &c, EntityID eid) {
    Component::Tilemap* tile = c.active_scene->getComponent<Component::Tilemap>(eid);
    if (tile == nullptr)
        return;
    
    Fresa::Gui::draw_vec2(tile->pos.x, tile->pos.y, "pos", eid, [&c, &tile, eid]() {
        Component::Collider* col = c.active_scene->getComponent<Component::Collider>(eid);
        if (col != nullptr and col->transform.pos() != tile->pos) {
            col->transform = tile->pos;
        }
        
        System::Tilemap::createVertices(c, tile);
    });
    ImGui::TableNextRow();
    
    
    Fresa::Gui::draw_vec2(size.x, size.y, "size", eid, [&c, &tile, eid]() {
        Vec2 prev_size = Vec2((int)tile->tiles[0].size(), (int)tile->tiles.size() - 1);
        
        //Delete X
        while (size.x < prev_size.x) {
            if (prev_size.x <= 1)
                break;
            
            for (int i = 0; i < tile->tiles.size() - 1; i++) {
                tile->tiles[i].pop_back();
            }
            prev_size.x--;
        }
        //Add X
        while (size.x > prev_size.x) {
            for (int i = 0; i < tile->tiles.size() - 1; i++) {
                tile->tiles[i].push_back(255);
            }
            prev_size.x++;
        }
        //Delete Y
        while (size.y < prev_size.y + 1) {
            if (prev_size.y <= 1)
                break;
            
            tile->tiles.pop_back();
            prev_size.y--;
        }
        //Add Y
        while (size.y > prev_size.y) {
            tile->tiles.push_back(std::vector<ui8>(tile->tiles[0].size(), 255));
            prev_size.y++;
        }
        
        Component::Collider* col = c.active_scene->getComponent<Component::Collider>(eid);
        if (col != nullptr)
            col->transform = Rect2(tile->pos, System::Tilemap::calculateSize(tile));
        
        System::Tilemap::createVertices(c, tile);
    });
    size = Vec2((int)tile->tiles[0].size(), (int)tile->tiles.size() - 1);
    ImGui::TableNextRow();
    
    
    Fresa::Gui::draw_vec2(tile->tex_size.x, tile->tex_size.y, "tex size", eid, [&c, &tile]() {
        System::Tilemap::createVertices(c, tile);
    });
    ImGui::TableNextRow();
    
    Fresa::Gui::draw_int(tile->layer, "layer", eid);
    ImGui::TableNextRow();
}
*/
#endif
