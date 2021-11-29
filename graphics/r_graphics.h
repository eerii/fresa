//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_api.h"
#include "fmath.h"

namespace Verse::Graphics
{
    bool init();
    bool update();
    bool stop();

    void onResize(Vec2<> size);

    DrawID getDrawID(const std::vector<VertexData> &vertices, const std::vector<ui16> &indices);
    DrawID getDrawID_Rect();
    DrawID getDrawID_Cube();

    void draw(const TextureData &tex, const DrawID draw_id, glm::mat4 model);
}

namespace Verse::Graphics::VerticesDefinitions {
    inline const std::vector<VertexData> cube_vertices = {
        {{-1.f, -1.f, -1.f}, Math::linear_color(0.701f, 0.839f, 0.976f)}, //Light
        {{1.f, -1.f, -1.f}, Math::linear_color(0.117f, 0.784f, 0.596f)}, //Teal
        {{1.f, 1.f, -1.f}, Math::linear_color(1.000f, 0.815f, 0.019f)}, //Yellow
        {{-1.f, 1.f, -1.f}, Math::linear_color(0.988f, 0.521f, 0.113f)}, //Orange
        {{-1.f, -1.f, 1.f}, Math::linear_color(0.925f, 0.254f, 0.345f)}, //Red
        {{1.f, -1.f, 1.f}, Math::linear_color(0.925f, 0.235f, 0.647f)}, //Pink
        {{1.f, 1.f, 1.f}, Math::linear_color(0.658f, 0.180f, 0.898f)}, //Purple
        {{-1.f, 1.f, 1.f}, Math::linear_color(0.258f, 0.376f, 0.941f)}, //Blue
    };

    inline const std::vector<ui16> cube_indices = {
        0, 3, 1, 3, 2, 1,
        1, 2, 5, 2, 6, 5,
        4, 7, 0, 7, 3, 0,
        3, 7, 2, 7, 6, 2,
        4, 0, 5, 0, 1, 5,
        5, 6, 4, 6, 7, 4,
    };

    inline const std::vector<VertexData> rect_vertices = {};
    inline const std::vector<ui16> rect_indices = {};
}
