//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "gui_entities.h"
#include "gui_types.h"
#include "r_textures.h"
#include "r_window.h"

#include "log.h"
#include "scene_list.h"
#include "s_collider.h"
#include "serialization.h"

using namespace Verse;

namespace {
    ui32 n = 0;
    std::map<str, std::function<void(Config&, EntityID)>> c_funcs;

    Component::Camera* prev_cam;
    Component::Camera* tile_cam;

    int scene_index = -1;
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

void c_collider(Config &c, EntityID e) {
    Component::Collider* col = c.active_scene->getComponent<Component::Collider>(e);
    
    Verse::Gui::draw_vec2(col->transform.x, col->transform.y, "pos", e);
    ImGui::TableNextRow();
    Verse::Gui::draw_vec2(col->transform.w, col->transform.h, "size", e);
    ImGui::TableNextRow();
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("layer");
    
    ImGui::TableSetColumnIndex(1);
    str layer_label = "##collayer" + std::to_string(e);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    ImGui::Combo(layer_label.c_str(), col->layer, System::Collider::layers_name);
}

void c_texture(Config &c, EntityID e) {
    Component::Texture* tex = c.active_scene->getComponent<Component::Texture>(e);
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("res");
    
    ImGui::TableSetColumnIndex(1);
    float line_height = ImGui::GetStyle().FramePadding.y * 2.0f + ImGui::CalcTextSize("X").y;
    ImVec2 button_size = { line_height + 3.0f, line_height };
    if (ImGui::Button("L", button_size))
        Graphics::Texture::loadTexture(tex->res, tex);
    
    ImGui::SameLine();
    str res_label = "##res" + std::to_string(e);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    ImGui::InputText(res_label.c_str(), &tex->res);
    
    ImGui::TableNextRow();
    
    Verse::Gui::draw_vec2(tex->transform.x, tex->transform.y, "pos", e);
    ImGui::TableNextRow();
    Verse::Gui::draw_vec2(tex->transform.w, tex->transform.h, "size", e);
    ImGui::TableNextRow();
    
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    if (ImGui::TreeNode("layers")) {
        ImGui::TableNextRow();
        
        for (int i = 0; i < tex->offset.size(); i++) {
            str label = "offset " + std::to_string(i);
            Verse::Gui::draw_vec2(tex->offset[i].x, tex->offset[i].y, label, e);
            ImGui::TableNextRow();
            
            Verse::Gui::draw_int(tex->layer[i], "layer " + std::to_string(i), e);
            ImGui::TableNextRow();
        }
        
        ImGui::TreePop();
    }
}

void c_animation(Config &c, EntityID e) {
    //Component::Animation* anim = c.active_scene->getComponent<Component::Animation>(e);
}

void c_tilemap(Config &c, EntityID e) {
    Component::Tilemap* tile = c.active_scene->getComponent<Component::Tilemap>(e);
    
    if (tile_cam == nullptr) {
        for (EntityID e : SceneView<Component::Camera>(*c.active_scene)) {
            if (c.active_scene->getName(e) != "free_camera")
                continue;
            tile_cam = c.active_scene->getComponent<Component::Camera>(e);
        }
    }
    
    ImGui::TableSetColumnIndex(0);
    if (c.tme_active and c.tme_curr_tmap == tile) {
        if (ImGui::SmallButton("save")) {
            c.tme_active = false;
            c.use_light = true;
            c.tme_curr_tmap = nullptr;
            c.tme_curr_id = 0;
            c.active_camera = prev_cam;
        }
    } else {
        if (ImGui::SmallButton("edit")) {
            c.tme_active = true;
            c.use_light = false;
            c.tme_curr_tmap = tile;
            c.tme_curr_id = e;
            prev_cam = c.active_camera;
            tile_cam->pos = prev_cam->pos;
            c.active_camera = tile_cam;
        }
    }
}

void c_actor(Config &c, EntityID e) {
    Component::Actor* actor = c.active_scene->getComponent<Component::Actor>(e);
    
    Verse::Gui::draw_vec2(actor->vel.x, actor->vel.y, "vel", e);
    ImGui::TableNextRow();
    Verse::Gui::draw_vec2(actor->remainder.x, actor->remainder.y, "remainder", e);
    ImGui::TableNextRow();
    
    Verse::Gui::draw_int(actor->max_move_speed, "max move speed", e);
    ImGui::TableNextRow();
    Verse::Gui::draw_int(actor->max_fall_speed, "max fall speed", e);
    ImGui::TableNextRow();
    
    Verse::Gui::draw_int(actor->acc_ground, "acc (ground)", e);
    ImGui::TableNextRow();
    Verse::Gui::draw_int(actor->friction_ground, "friction (ground)", e);
    ImGui::TableNextRow();
    
    Verse::Gui::draw_bool(actor->has_gravity, "gravity", e);
    ImGui::TableNextRow();
    
    //TODO: Controller
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("collision mask");
    ImGui::TableSetColumnIndex(1);
    int i = 0;
    for (str layer : System::Collider::layers_name) {
        str layer_label = layer + "##" + std::to_string(e);
        bool layer_active = actor->collision_mask[i];
        
        ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
        if (ImGui::Selectable(layer_label.c_str(), layer_active))
            (actor->collision_mask[i]) ? actor->collision_mask.reset(i) : actor->collision_mask.set(i);
        i++;
    }
}

void c_light(Config &c, EntityID e) {
    Component::Light* light = c.active_scene->getComponent<Component::Light>(e);
    
    Verse::Gui::draw_vec2(light->pos.x, light->pos.y, "pos", e);
    ImGui::TableNextRow();
    
    Verse::Gui::draw_float(light->radius, "radius", e);
}

void c_camera(Config &c, EntityID e) {
    Component::Camera* cam = c.active_scene->getComponent<Component::Camera>(e);
    
    Verse::Gui::draw_vec2(cam->target_pos.x, cam->target_pos.y, "target pos", e);
    ImGui::TableNextRow();
    
    Verse::Gui::draw_vec2(cam->pos.x, cam->pos.y, "pos", e);
    ImGui::TableNextRow();
    
    Verse::Gui::draw_vec2(cam->vel.x, cam->vel.y, "vel", e);
    ImGui::TableNextRow();
    
    Verse::Gui::draw_vec2(cam->remainder.x, cam->remainder.y, "remainder", e);
    ImGui::TableNextRow();
    
    //TODO: Controller, bounds
}

void c_fire(Config &c, EntityID e) {
    Component::Fire* fire = c.active_scene->getComponent<Component::Fire>(e);
    
    Verse::Gui::draw_vec2(fire->transform.x, fire->transform.y, "pos", e);
    ImGui::TableNextRow();
    Verse::Gui::draw_vec2(fire->transform.w, fire->transform.h, "size", e); //TODO: Change fire data size
    ImGui::TableNextRow();
    Verse::Gui::draw_vec2(fire->offset.x, fire->offset.y, "offset", e);
    ImGui::TableNextRow();
    
    Verse::Gui::draw_float(fire->freq, "freq", e);
    ImGui::TableNextRow();
    Verse::Gui::draw_vec2(fire->dir.x, fire->dir.y, "dir", e);
    ImGui::TableNextRow();
    
    Verse::Gui::draw_int(fire->layer, "layer", e);
    ImGui::TableNextRow();
    Verse::Gui::draw_ui8(fire->fps, "fps", e);
    ImGui::TableNextRow();
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("texture");
    
    ImGui::TableSetColumnIndex(1);
    float line_height = ImGui::GetStyle().FramePadding.y * 2.0f + ImGui::CalcTextSize("X").y;
    ImVec2 button_size = { line_height + 3.0f, line_height };
    if (ImGui::Button("L", button_size))
        Graphics::Texture::loadTexture(fire->flame_tex_res, fire->flame_tex);
    
    ImGui::SameLine();
    str res_label = "##res" + std::to_string(e);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    ImGui::InputText(res_label.c_str(), &fire->flame_tex_res);
}

void c_scene_transition(Config &c, EntityID e) {
    Component::SceneTransition* trans = c.active_scene->getComponent<Component::SceneTransition>(e);
    
    if (scene_index == -1) {
        auto it = std::find(scenes.begin(), scenes.end(), trans->scene_name);
        if (it != scenes.end())
            scene_index = 0;
        else
            scene_index = (int)std::distance(scenes.begin(), it);
    }
    
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("new scene");
    
    ImGui::TableSetColumnIndex(1);
    str layer_label = "##scenetrans" + std::to_string(e);
    ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
    if (ImGui::Combo(layer_label.c_str(), &scene_index, scenes) and trans->scene_name != scenes[scene_index]) {
        trans->scene_name = scenes[scene_index];
        trans->to_scene = new Scene();
        Serialization::loadScene(trans->scene_name, trans->to_scene, c);
    }
    ImGui::TableNextRow();
    
    Verse::Gui::draw_vec2(trans->to_pos.x, trans->to_pos.y, "new scene pos", e);
}

void c_player(Config &c, EntityID e) {
    //Component::Player* player = c.active_scene->getComponent<Component::Player>(e);
}

void components(Config &c, Signature mask, EntityID e) {
    if (c_funcs.size() == 0) {
        c_funcs["collider"] = c_collider;
        c_funcs["texture"] = c_texture;
        c_funcs["animation"] = c_animation;
        c_funcs["tilemap"] = c_tilemap;
        c_funcs["actor"] = c_actor;
        c_funcs["light"] = c_light;
        c_funcs["camera"] = c_camera;
        c_funcs["fire"] = c_fire;
        c_funcs["sceneTransition"] = c_scene_transition;
        c_funcs["player"] = c_player;
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
