# Window

`graphics/r_window`

Data:

```cpp
struct WindowData {
    SDL_Window* window;
    str name;
    Vec2<ui16> size;
    float dpi;
    ui16 refresh_rate;
    bool vsync;
    Event::Observer resize_observer;
}
```

```cpp
struct WindowTransform {
    glm::mat4 model;
    glm::mat4 projection;
}
```

```cpp
inline Event::Event<Vec2<ui16>> event_window_resize;
```

Functions:

```cpp
WindowData create(Vec2<ui16> size, str name, WindowData* previous = nullptr)
```

```cpp
float getDPI(SDL_Window* win)
```

```cpp
ui16 getRefreshRate(SDL_Window* win)
```

```cpp
WindowTransform getScaledTransform(Vec2<ui16> resolution)
```

```cpp
void onResize(Vec2<ui16> size)
```