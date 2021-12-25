//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#ifndef DISABLE_GUI

#include "gui.h"
#include "config.h"
#include "input.h"
#include "r_graphics.h"

using namespace Fresa;

#ifdef USE_VULKAN
namespace {
    VkDescriptorPool gui_descriptor_pool;
}
#endif

bool Gui::ActiveWindows::test = true;

void Gui::init(const Graphics::GraphicsAPI &api, const Graphics::WindowData &win) {
    //---Initialization---
    ImGui::CreateContext();
    
    #if defined USE_OPENGL
    if (not ImGui_ImplSDL2_InitForOpenGL(win.window, api.context))
        log::error("Error initializing ImGui for SDL");
    if (not ImGui_ImplOpenGL3_Init())
        log::error("Error initializing ImGui for OpenGL");
    #elif defined USE_VULKAN
    if (not ImGui_ImplSDL2_InitForVulkan(win.window))
        log::error("Error initializing ImGui for SDL");
    
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    create_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    create_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    create_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(api.device, &create_info, nullptr, &gui_descriptor_pool) != VK_SUCCESS)
        log::error("Failed to create the GUI descriptor pool");
    
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = api.instance;
    init_info.PhysicalDevice = api.physical_device;
    init_info.Device = api.device;
    init_info.QueueFamily = api.cmd.queue_indices.graphics.value();
    init_info.Queue = api.cmd.queues.graphics;
    init_info.DescriptorPool = gui_descriptor_pool;
    init_info.MinImageCount = api.render.swapchain.min_image_count;
    init_info.ImageCount = api.render.swapchain.size;
    init_info.Allocator = nullptr;
    
    if (not ImGui_ImplVulkan_Init(&init_info, api.render.render_pass))
        log::error("Error initializing ImGui for OpenGL");
    
    //: Transfer fonts
    VkCommandBuffer command_buffer = Graphics::VK::beginSingleUseCommandBuffer(api.device, api.cmd.command_pools.at("transfer"));
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    Graphics::VK::endSingleUseCommandBuffer(api.device, command_buffer, api.cmd.command_pools.at("transfer"), api.cmd.queues.transfer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    #endif
    
    //---IO---
    io = &ImGui::GetIO();
    
    //: Docking
    //  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    //---Style---
    style = &ImGui::GetStyle();
    
    //: Borders
    style->WindowBorderSize = 0;
    style->ChildBorderSize = 0;
    style->PopupBorderSize = 0;
    
    //: Rounding
    style->WindowRounding = 8;
    style->ChildRounding = 8;
    style->PopupRounding = 8;
    style->FrameRounding = 6;
    
    //: Colors
    ImVec3 color_for_text = ImVec3(254.f / 255.f, 214.f / 255.f, 173.f / 255.f);
    ImVec3 color_for_head = ImVec3(92.f / 255.f, 133.f / 255.f, 219.f / 255.f);
    ImVec3 color_for_area = ImVec3(42.f / 255.f, 67.f / 255.f, 151.f / 255.f);
    ImVec3 color_for_body = ImVec3(1.f / 255.f, 9.f / 255.f, 33.f / 255.f);
    ImVec3 color_for_pops = ImVec3(1.f / 255.f, 9.f / 255.f, 33.f / 255.f);
    setColors(color_for_text, color_for_head, color_for_area, color_for_body, color_for_pops);
    
    //---Resize---
    Graphics::event_window_resize.callback([](const Vec2<> &size){
        io->DisplaySize.x = (float)size.x;
        io->DisplaySize.y = (float)size.y;
    });
}

void Gui::GuiSystem::update() {
    //: Delta
    io->DeltaTime = Config::timestep * Config::game_speed;
    
    //: Keyboard and mouse
    Input::gui_using_mouse = io->WantCaptureMouse;
    Input::gui_using_keyboard = io->WantCaptureKeyboard;
}

void Gui::GuiSystem::render() {
    #if defined USE_OPENGL
    ImGui_ImplOpenGL3_NewFrame();
    #elif defined USE_VULKAN
    ImGui_ImplVulkan_NewFrame();
    #endif

    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    
    ImGui::Render();
}

void Gui::GuiSystem::present() {
    #if defined USE_OPENGL
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    #elif defined USE_VULKAN
    
    #endif
}

void Gui::setColors(ImVec3 text, ImVec3 head, ImVec3 area, ImVec3 body, ImVec3 pops) {
    style->Colors[ImGuiCol_Text] = ImVec4( text.x, text.y, text.z, 1.00f );
    style->Colors[ImGuiCol_TextDisabled] = ImVec4( text.x, text.y, text.z, 0.58f );
    style->Colors[ImGuiCol_WindowBg] = ImVec4( body.x, body.y, body.z, 0.95f );
    style->Colors[ImGuiCol_Border] = ImVec4( body.x, body.y, body.z, 0.00f );
    style->Colors[ImGuiCol_BorderShadow] = ImVec4( body.x, body.y, body.z, 0.00f );
    style->Colors[ImGuiCol_FrameBg] = ImVec4( area.x, area.y, area.z, 1.00f );
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4( head.x, head.y, head.z, 0.78f );
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_TitleBg] = ImVec4( area.x, area.y, area.z, 1.00f );
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4( area.x, area.y, area.z, 0.75f );
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4( area.x, area.y, area.z, 0.47f );
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4( area.x, area.y, area.z, 1.00f );
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4( head.x, head.y, head.z, 0.21f );
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( head.x, head.y, head.z, 0.78f );
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_CheckMark] = ImVec4( head.x, head.y, head.z, 0.80f );
    style->Colors[ImGuiCol_SliderGrab] = ImVec4( head.x, head.y, head.z, 0.50f );
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_Button] = ImVec4( head.x, head.y, head.z, 0.50f );
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4( head.x, head.y, head.z, 0.86f );
    style->Colors[ImGuiCol_ButtonActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_Header] = ImVec4( head.x, head.y, head.z, 0.76f );
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4( head.x, head.y, head.z, 0.86f );
    style->Colors[ImGuiCol_HeaderActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4( head.x, head.y, head.z, 0.15f );
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4( head.x, head.y, head.z, 0.78f );
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_PlotLines] = ImVec4( text.x, text.y, text.z, 0.63f );
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4( text.x, text.y, text.z, 0.63f );
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4( head.x, head.y, head.z, 1.00f );
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4( head.x, head.y, head.z, 0.43f );
    style->Colors[ImGuiCol_PopupBg] = ImVec4( pops.x, pops.y, pops.z, 0.92f );
}

#endif
