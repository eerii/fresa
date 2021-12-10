//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#ifndef DISABLE_GUI

#include "gui_performance.h"
#include "ftime.h"
#include <numeric>

using namespace Fresa;

namespace {
    TimerID timer;
    bool initialized;

    std::array<double, 3> adjusted_physics_time;
    std::array<double, 3> adjusted_render_time;

    std::array<double, 10> points10_physics;
    std::array<double, 100> points100_physics;
    std::array<double, 10> points10_render;
    std::array<double, 100> points100_render;
    int j = 0; int k = 0;

    double max_physics;
    double min_physics = 1e10;
    double max_render;
    double min_render = 1e10;

    float x[100];
}

void Gui::performance(Config &c) {
    ImGui::Begin("performance");
    
    if (not initialized) {
        initialized = true;
        timer = setTimer(100);
        
        for (int i = 0; i < 100; i++)
            x[i] = i * 0.1f;
    }
    
    if (checkTimer(timer)) {
        timer = setTimer(100);
        
        double pt = c.physics_time * 1e-6;
        double rt = c.render_time * 1e-6;
        
        adjusted_physics_time[0] = pt;
        adjusted_render_time[0] = rt;
        
        points10_physics[j] = pt;
        points100_physics[k] = pt;
        points10_render[j] = rt;
        points100_render[k] = rt;
        
        max_physics = std::max(pt, max_physics);
        min_physics = std::min(pt, min_physics);
        max_render = std::max(rt, max_render);
        min_render = std::min(rt, min_render);
        
        j++;
        k++;
        
        if (j == points10_physics.size()) {
            j = 0;
            
            adjusted_physics_time[1] = 0;
            for (double p : points10_physics)
                adjusted_physics_time[1] += p;
            adjusted_physics_time[1] /= points10_physics.size();
            
            adjusted_render_time[1] = 0;
            for (double p : points10_render)
                adjusted_render_time[1] += p;
            adjusted_render_time[1] /= points10_render.size();
        }
        
        if (k == points100_physics.size()) {
            k = 0;
            
            max_physics = 0; max_render = 0;
            min_physics = 1e10; min_render = 1e10;
            
            adjusted_physics_time[2] = 0;
            for (double p : points100_physics)
                adjusted_physics_time[2] += p;
            adjusted_physics_time[2] /= points100_physics.size();
            
            adjusted_render_time[2] = 0;
            for (double p : points100_render)
                adjusted_render_time[2] += p;
            adjusted_render_time[2] /= points100_render.size();
        }
    }
    
    ImGui::Text("physics time");
    ImGui::Text("0.1s: %f ms", adjusted_physics_time[0]);
    ImGui::Text("1s: %f ms", adjusted_physics_time[1]);
    ImGui::Text("10s: %f ms", adjusted_physics_time[2]);
    ImGui::Text("max: %f ms", max_physics);
    ImGui::Text("min: %f ms", min_physics);
    
    ImGui::Text("");
    
    ImGui::Text("render time");
    ImGui::Text("0.1s: %f ms", adjusted_render_time[0]);
    ImGui::Text("1s: %f ms", adjusted_render_time[1]);
    ImGui::Text("10s: %f ms", adjusted_render_time[2]);
    ImGui::Text("max: %f ms", max_render);
    ImGui::Text("min: %f ms", min_render);
    
    ImGui::Text("");
    
    ImGui::Text("total time");
    ImGui::Text("0.1s: %f ms", adjusted_physics_time[0] + adjusted_render_time[0]);
    ImGui::Text("1s: %f ms", adjusted_physics_time[1] + adjusted_render_time[1]);
    ImGui::Text("10s: %f ms", adjusted_physics_time[2] + adjusted_render_time[2]);
    ImGui::Text("max: %f ms", max_physics + max_render);
    ImGui::Text("min: %f ms", min_physics + min_render);
    
    /*if (ImPlot::BeginPlot("performance")) {
        float y[100];
        
        ImPlot::SetupAxesLimits(0, 10, 0.0, 16.6, ImGuiCond_Always);
        
        for (int i = 0; i < 100; i++)
            y[i] = points100_render[i] + points100_physics[i];
        ImPlot::PlotShaded("render", x, y, 100);
        
        for (int i = 0; i < 100; i++)
            y[i] = points100_physics[i];
        ImPlot::PlotShaded("physics", x, y, 100);
        
        ImPlot::EndPlot();
    }*/
    
    ImGui::End();
}

#endif
