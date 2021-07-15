//project verse, 2017-2021
//by jose pazos perez
//all rights reserved uwu

#ifdef USE_VULKAN

#include "r_renderer.h"
#include "r_vulkan_core.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "log.h"

using namespace Verse;
using namespace Graphics;
using namespace glm;

namespace {
    Vulkan vulkan;
}

void Graphics::Renderer::create(Config &c) {
    VK::initVulkan(&vulkan, c);
}



void Graphics::Renderer::renderTexture(Config &c, TextureData &data) {
    
}

void Graphics::Renderer::renderTilemap(Config &c, TextureData &data) {
    
}

void Graphics::Renderer::renderNoise(Config &c, TextureData &mask_data, TextureData &noise_data) {
    
}

void Graphics::Renderer::renderText(Config &c, TextureData &data, float r, float g, float b, bool same_color) {
    
}


void Graphics::Renderer::renderPost(Config &c) {
    
}

void Graphics::Renderer::renderCam(Config &c) {
    
}

void Graphics::Renderer::renderWindow(Config &c) {
    
}



void Graphics::Renderer::renderTest(Config &c) {
    vulkan.renderFrame(c);
}

void Graphics::Renderer::renderDebugCollider(Config &c, Rect2 col, bool colliding) {
    
}



void Graphics::Renderer::present(SDL_Window *window) {
    
}

void Graphics::Renderer::clear(Config &c) {
    
}



void Graphics::Renderer::destroy() {
    vulkan.destroy();
}



void Graphics::Renderer::onResize(Config &c) {
    vulkan.recreateSwapchain(c);
}



void Graphics::Renderer::toggleDepthTest(bool enable) {
    
}



glm::mat4 Graphics::Renderer::matModel2D(Vec2 pos, Vec2 size, float rotation) {
    return glm::mat4(0);
}



glm::mat4 Graphics::Renderer::matModel2D(Rect2 rect, float rotation) {
    return glm::mat4(0);
}


#endif
