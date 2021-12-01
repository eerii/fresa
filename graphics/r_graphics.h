//project fresa, 2017-2022
//by jose pazos perez
//all rights reserved uwu

#pragma once

#include "r_api.h"

namespace Fresa::Graphics
{
    bool init();
    bool update();
    bool stop();

    void onResize(Vec2<> size);

    DrawID getDrawID(const std::vector<VertexData> &vertices, const std::vector<ui16> &indices, DrawShaders shader = SHADER_DRAW);
    DrawID getDrawID_Rect(DrawShaders shader = SHADER_DRAW);
    DrawID getDrawID_Cube(DrawShaders shader = SHADER_DRAW);

    TextureID getTextureID(str path, Channels ch = TEXTURE_CHANNELS_RGBA);
    void bindTexture(DrawID draw_id, TextureID texture_id);
    void unbindTexture(DrawID draw_id);

    void draw(const DrawID draw_id, glm::mat4 model);
}

namespace Fresa::Graphics::VerticesDefinitions {
    inline const std::vector<VertexData> cube_vertices = {
        {{-1.f, -1.f, -1.f}, {0.701f, 0.839f, 0.976f}, {0.0f, 0.0f}}, //Light
        {{1.f, -1.f, -1.f}, {0.117f, 0.784f, 0.596f}, {0.0f, 0.0f}}, //Teal
        {{1.f, 1.f, -1.f}, {1.000f, 0.815f, 0.019f}, {0.0f, 0.0f}}, //Yellow
        {{-1.f, 1.f, -1.f}, {0.988f, 0.521f, 0.113f}, {0.0f, 0.0f}}, //Orange
        {{-1.f, -1.f, 1.f}, {0.925f, 0.254f, 0.345f}, {0.0f, 0.0f}}, //Red
        {{1.f, -1.f, 1.f}, {0.925f, 0.235f, 0.647f}, {0.0f, 0.0f}}, //Pink
        {{1.f, 1.f, 1.f}, {0.658f, 0.180f, 0.898f}, {0.0f, 0.0f}}, //Purple
        {{-1.f, 1.f, 1.f}, {0.258f, 0.376f, 0.941f}, {0.0f, 0.0f}}, //Blue
    };

    inline const std::vector<ui16> cube_indices = {
        0, 3, 1, 3, 2, 1,
        1, 2, 5, 2, 6, 5,
        4, 7, 0, 7, 3, 0,
        3, 7, 2, 7, 6, 2,
        4, 0, 5, 0, 1, 5,
        5, 6, 4, 6, 7, 4,
    };

    inline const std::vector<VertexData> rect_vertices = {
        {{-1.f, -1.f, 0.f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.f, -1.f, 0.f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.f, 1.f, 0.f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-1.f, 1.f, 0.f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    };
    inline const std::vector<ui16> rect_indices = {
        0, 1, 3, 2, 3, 1
    };
}
