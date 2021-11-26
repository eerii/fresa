//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#include "r_api.h"
#include "log.h"

using namespace Verse;
using namespace Graphics;

DrawID API::registerDrawData(GraphicsAPI &api, const std::vector<VertexData> &vertices, const std::vector<ui16> &indices) {
    //TODO: Convert into a more efficient pool allocator
    static DrawID id = 0;
    while (draw_data.find(id) != draw_data.end())
        id++;
    draw_data[id] = API::createDrawData(api, vertices, indices);
    return id;
}

void API::draw(const TextureData &tex, DrawID draw_id, VK::UniformBufferObject ubo) {
    const DrawData* data = &draw_data.at(draw_id);
    if (data == nullptr)
        log::error("Tried to draw an object with a null pointer draw data");
    draw_queue[&tex].push_back(std::pair(data, ubo));
}
