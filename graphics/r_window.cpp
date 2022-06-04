//: fresa by jose pazos perez, licensed under GPLv3
#include "r_window.h"
#include "r_api.h"
#include "log.h"

//: SDL Window Flags
#if defined USE_VULKAN
    #define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
    #define RENDERER_NAME "vulkan"
#elif defined USE_OPENGL
    #define W_FLAGS SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    #define RENDERER_NAME "opengl"
#endif

using namespace Fresa;
using namespace Graphics;

//---------------------------------------------------
//: Create window
//      Can optionally take a WindowData* with previous data
//      In that case, SDL_Window* is copied and the old resize event observers are propperly destroyed
//---------------------------------------------------
WindowData Window::create(Vec2<ui16> size, str name, WindowData* previous) {
    WindowData win;
    
    if (previous == nullptr) {
        //: SDL window
        #ifdef __EMSCRIPTEN__
            SDL_Renderer* renderer = nullptr;
            SDL_CreateWindowAndRenderer(size.x, size.y, SDL_WINDOW_OPENGL, &win.window, &renderer);
            
            if (win.window == nullptr or renderer == nullptr)
                log::error("Failed to create a SDL_Window and a SDL_Renderer from Window::create()", SDL_GetError());
        #else
            win.window = SDL_CreateWindow((name + " - " + RENDERER_NAME).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, W_FLAGS);
            
            if (win.window == nullptr)
                log::error("Failed to create a SDL_Window from Window::create()", SDL_GetError());
            
            SDL_SetWindowResizable(win.window, SDL_TRUE);
            SDL_SetWindowMinimumSize(win.window, 256, 180);
        #endif
    } else {
        //: Copy SDL window
        win.window = previous->window;
        
        //: Clean event observer
        previous->resize_observer.reset();
    }
    
    //: Window size
    win.size = size;
    
    //: Window name
    win.name = name;
    
    //: DPI
    win.dpi = getDPI(win.window);
    
    //: Refresh rate
    win.refresh_rate = getRefreshRate(win.window);
    log::graphics("Refresh Rate: %d", win.refresh_rate);
    
    //: V-Sync
    win.vsync = true;
    
    //: Event
    win.resize_observer = event_window_resize.createObserver(onResize);
    
    return win;
}

//---------------------------------------------------
//: Get DPI
//---------------------------------------------------
float Window::getDPI(SDL_Window* win) {
    //: Display index
    int display_index = SDL_GetWindowDisplayIndex(win);
    
    //: DPI
    float ddpi = 0.0f;
    SDL_GetDisplayDPI(display_index, &ddpi, nullptr, nullptr);
    return ddpi / 96.0f;
}

//---------------------------------------------------
//: Get refresh rate
//---------------------------------------------------
ui16 Window::getRefreshRate(SDL_Window* win) {
    //: Display index
    int display_index = SDL_GetWindowDisplayIndex(win);
    
    //: Display mode
    SDL_DisplayMode mode;
    if(SDL_GetDisplayMode(display_index, 0, &mode))
        log::error("Failed to get a display mode for SDL_Window", SDL_GetError());
    
    return (ui16)mode.refresh_rate;
}

//---------------------------------------------------
//: Get window buffer object for scaled perspective
//      - Needs window to be create first
//---------------------------------------------------
WindowTransform Window::getScaledTransform(Vec2<ui16> resolution) {
    WindowTransform transform{};
    
    //: Calculate scaled resolution
    Vec2<float> ratios = window.size.to<float>() / resolution.to<float>();
    ui16 scale = (ratios.x < ratios.y) ? floor(ratios.x) : floor(ratios.y);
    Vec2<ui16> scaled_resolution = resolution * scale;
    
    //: Model
    transform.model = glm::scale(glm::mat4(1.0f), glm::vec3((float)scaled_resolution.x, (float)scaled_resolution.y, 1.0f));
    
    //: Projection (Fixes coordinates going from -1 to 1
    transform.projection = glm::ortho((float)-window.size.x, (float)window.size.x, (float)-window.size.y, (float)window.size.y);
    
    return transform;
}

//---------------------------------------------------
//: Resize callback
//---------------------------------------------------
void Window::onResize(Vec2<ui16> size) {
    //: Update window data
    window = create(size, window.name, &window);
    
    //: Pass resize to API
    resize();
}
