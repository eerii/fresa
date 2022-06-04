//: fresa by jose pazos perez, licensed under GPLv3
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
    
    std::vector<std::array<double, 300>> physics_systems_points{};
    std::vector<std::array<double, 3>> physics_systems_averages{};
    
    std::vector<std::array<double, 300>> render_systems_points{};
    std::vector<std::array<double, 3>> render_systems_averages{};
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
        init = true;
    }
    
    fps_points[current] = 1.0 / sec(time() - t);
    physics_frame_points[current] = Performance::physics_frame_time;
    physics_iteration_points[current] = Performance::physics_iteration_time;
    physics_event_points[current] = Performance::physics_event_time;
    
    for (int i = 0; i < Performance::physics_system_time.size(); i++)
        physics_systems_points.at(i)[current] = Performance::physics_system_time.at(i);
    for (int i = 0; i < Performance::render_system_time.size(); i++)
        render_systems_points.at(i)[current] = Performance::render_system_time.at(i);
    
    current = (current + 1) % 300;
    t = time();
    
    if (checkTimer(timer)) {
        timer = setTimer(100);
        
        updateAverages(fps_points, fps_averages, current);
        updateAverages(physics_frame_points, physics_frame_averages, current);
        updateAverages(physics_iteration_points, physics_iteration_averages, current);
        updateAverages(physics_event_points, physics_event_averages, current);
        
        for (int i = 0; i < physics_systems_points.size(); i++)
            updateAverages(physics_systems_points.at(i), physics_systems_averages.at(i), current);
        for (int i = 0; i < render_systems_points.size(); i++)
            updateAverages(render_systems_points.at(i), render_systems_averages.at(i), current);
    }
    
    ImGui::Begin("performance");
    
    ImGui::Text("fps:    %6.1f   %6.1f   %6.1f", fps_averages.at(0), fps_averages.at(1), fps_averages.at(2));
    
    ImGui::Text("");
    
    ImGui::Text("physics time");
    ImGui::Text("frame:  %6.3f   %6.3f   %6.3f", physics_frame_averages.at(0), physics_frame_averages.at(1), physics_frame_averages.at(2));
    ImGui::Text("iter:   %6.3f   %6.3f   %6.3f", physics_iteration_averages.at(0), physics_iteration_averages.at(1), physics_iteration_averages.at(2));
    ImGui::Text("event:  %6.3f   %6.3f   %6.3f", physics_event_averages.at(0), physics_event_averages.at(1), physics_event_averages.at(2));
    
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
    
    ImGui::End();
}

#endif
