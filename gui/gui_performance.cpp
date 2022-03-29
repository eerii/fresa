//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3

#ifndef DISABLE_GUI

#include "gui.h"
#include "ecs.h"
#include "f_time.h"
#include "r_graphics.h"

using namespace Fresa;

namespace {
    std::array<double, 300> fps_points{};
    std::array<double, 3> fps_averages{};
    
    std::array<double, 300> physics_frame_points{};
    std::array<double, 3> physics_frame_averages{};
    
    std::array<double, 300> physics_iteration_points{};
    std::array<double, 3> physics_iteration_averages{};
    
    std::array<double, 300> physics_event_points{};
    std::array<double, 3> physics_event_averages{};
    
    std::array<double, 300> render_frame_points{};
    std::array<double, 3> render_frame_averages{};
    
    std::array<double, 300> render_draw_points{};
    std::array<double, 3> render_draw_averages{};
    
    std::vector<std::array<double, 300>> physics_systems_points{};
    std::vector<std::array<double, 3>> physics_systems_averages{};
    
    std::vector<std::array<double, 300>> render_systems_points{};
    std::vector<std::array<double, 3>> render_systems_averages{};
    
    #ifdef USE_VULKAN
    std::vector<std::array<double, 300>> render_draw_shader_points{};
    std::vector<std::array<double, 3>> render_draw_shader_averages{};
    #endif
}

double average(std::array<double, 300> &points, int current, int frames) {
    double avg = 0;
    for (int i = current - frames; i < current; i++)
        avg += points.at((i + points.size()) % points.size());
    return avg / (double)frames;
}

void updateAverages(std::array<double, 300> &points, std::array<double, 3> &averages, int current) {
    averages = {
        average(points, current, 12), //: 0.2s
        average(points, current, 60), //: 1.0s
        average(points, current, 300) //: 5.0s
    };
}

#ifdef USE_VULKAN
double timeFromTimestamp(ui64 t1, ui64 t2) {
    return ((double)(t2 - t1) / (double)Performance::timestamp_period) * 1.0e-6; //: Time in ms
}
#endif

void Gui::win_performance() {
    static int current = 0;
    static TimerID timer = setTimer(100);
    static Clock::time_point t = time();
    static bool init = false;
    
    if (not init) {
        physics_systems_points.resize(System::physics_update_systems.size());
        physics_systems_averages.resize(System::physics_update_systems.size());
        render_systems_points.resize(System::render_update_systems.size());
        render_systems_averages.resize(System::render_update_systems.size());
        #ifdef USE_VULKAN
        render_draw_shader_points.resize(Graphics::api.shaders.size());
        render_draw_shader_averages.resize(Graphics::api.shaders.size());
        #endif
        init = true;
    }
    
    fps_points[current] = 1.0 / sec(time() - t);
    physics_frame_points[current] = Performance::physics_frame_time;
    physics_iteration_points[current] = Performance::physics_iteration_time;
    physics_event_points[current] = Performance::physics_event_time;
    render_frame_points[current] = Performance::render_frame_time;
    
    for (int i = 0; i < Performance::physics_system_time.size(); i++)
        physics_systems_points.at(i)[current] = Performance::physics_system_time.at(i);
    for (int i = 0; i < Performance::render_system_time.size(); i++)
        render_systems_points.at(i)[current] = Performance::render_system_time.at(i);
    
    #if defined USE_OPENGL
    render_draw_points[current] = Performance::render_draw_time;
    #elif defined USE_VULKAN
    if (Performance::timestamps.size() == 0) {
        log::warn("GPU Timestamps are not initilized...");
    } else if (Performance::timestamp_period > 0) {
        ui32 time_points = (ui32)Graphics::api.shaders.size() + 1;
        ui32 swapchain_size = (ui32)Performance::timestamps.size() / (time_points * 2);
        
        render_draw_points[current] = 0;
        for (int i = 0; i < swapchain_size; i++)
            render_draw_points[current] += timeFromTimestamp(Performance::timestamps.at(i * time_points * 2),
                                                             Performance::timestamps.at(i * time_points * 2 + 1));
        render_draw_points[current] /= swapchain_size;
        
        for (int j = 0; j < Graphics::api.shaders.size(); j++) {
            render_draw_shader_points.at(j)[current] = 0;
            for (int i = 0; i < swapchain_size; i++)
                render_draw_shader_points.at(j)[current] += timeFromTimestamp(Performance::timestamps.at((i * time_points + j) * 2),
                                                                              Performance::timestamps.at((i * time_points + j) * 2 + 1));
        }
    }
    #endif
    
    current = (current + 1) % 300;
    t = time();
    
    if (checkTimer(timer)) {
        timer = setTimer(100);
        
        updateAverages(fps_points, fps_averages, current);
        updateAverages(physics_frame_points, physics_frame_averages, current);
        updateAverages(physics_iteration_points, physics_iteration_averages, current);
        updateAverages(physics_event_points, physics_event_averages, current);
        updateAverages(render_frame_points, render_frame_averages, current);
        updateAverages(render_draw_points, render_draw_averages, current);
        
        for (int i = 0; i < physics_systems_points.size(); i++)
            updateAverages(physics_systems_points.at(i), physics_systems_averages.at(i), current);
        for (int i = 0; i < render_systems_points.size(); i++)
            updateAverages(render_systems_points.at(i), render_systems_averages.at(i), current);
        
        #ifdef USE_VULKAN
        for (int i = 0; i < render_draw_shader_points.size(); i++)
            updateAverages(render_draw_shader_points.at(i), render_draw_shader_averages.at(i), current);
        #endif
    }
    
    ImGui::Begin("performance");
    
    ImGui::Text("fps:    %6.1f   %6.1f   %6.1f", fps_averages.at(0), fps_averages.at(1), fps_averages.at(2));
    
    ImGui::Text("");
    
    ImGui::Text("physics time");
    ImGui::Text("frame:  %6.3f   %6.3f   %6.3f", physics_frame_averages.at(0), physics_frame_averages.at(1), physics_frame_averages.at(2));
    ImGui::Text("iter:   %6.3f   %6.3f   %6.3f", physics_iteration_averages.at(0), physics_iteration_averages.at(1), physics_iteration_averages.at(2));
    ImGui::Text("event:  %6.3f   %6.3f   %6.3f", physics_event_averages.at(0), physics_event_averages.at(1), physics_event_averages.at(2));
    
    ImGui::Text("");
    
    ImGui::Text("render time");
    ImGui::Text("frame:  %6.3f   %6.3f   %6.3f", render_frame_averages.at(0), render_frame_averages.at(1), render_frame_averages.at(2));
    ImGui::Text("draw:   %6.3f   %6.3f   %6.3f", render_draw_averages.at(0), render_draw_averages.at(1), render_draw_averages.at(2));
    
    ImGui::Text("");
    
    if (ImGui::CollapsingHeader("physic systems")) {
        int i = 0;
        for (auto &[priority, system] : System::physics_update_systems) {
            ImGui::Text("%s: %6.3f   %6.3f   %6.3f", str(system.first).c_str(),
                        physics_systems_averages.at(i).at(0), physics_systems_averages.at(i).at(1), physics_systems_averages.at(i).at(2)); i++;
        }
    }
    
    if (ImGui::CollapsingHeader("render systems")) {
        int i = 0;
        for (auto &[priority, system] : System::render_update_systems) {
            ImGui::Text("%s: %6.3f   %6.3f   %6.3f", str(system.first).c_str(),
                        render_systems_averages.at(i).at(0), render_systems_averages.at(i).at(1), render_systems_averages.at(i).at(2)); i++;
        }
    }
    
    #ifdef USE_VULKAN
    if (ImGui::CollapsingHeader("gpu timers")) {
        int i = 0;
        for (auto &[shader, data] : Graphics::api.shaders) {
            ImGui::Text("%s: %6.3f   %6.3f   %6.3f", shader.c_str(),
                        render_draw_shader_averages.at(i).at(0), render_draw_shader_averages.at(i).at(1), render_draw_shader_averages.at(i).at(2)); i++;
        }
    }
    #endif
    
    ImGui::End();
}

#endif
