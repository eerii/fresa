//project fresa, 2017-2022
//by jose pazos perez
//licensed under GPLv3 uwu

#pragma once

#include "r_vulkan_api.h"
#include "r_opengl_api.h"

//---Graphics---
//      This is fresa's API for graphics, what is meant to be used when designing games
//      It is intended to be as straightforward as possible, allowing for texture and vertex data registration for later use
//      Read more about each specific function in the .cpp file

namespace Fresa::Graphics
{
    bool init();
    bool update();
    bool stop();
    
    template <typename... UBO, typename V, typename I, std::enable_if_t<Reflection::is_reflectable<V> && std::is_integral_v<I>, bool> = true>
    DrawDescription getDrawDescription(const std::vector<V> &vertices, const std::vector<I> &indices,
                                       ShaderID shader, TextureID texture = no_texture, bool call_from_instanced = false) {
        
        if (api.shaders.at(shader).is_instanced and not call_from_instanced)
            log::error("You are getting a draw description for an instanced shader using the function for regular rendering, use getDrawDescriptionI()");
        
        if (api.shaders.at(shader).is_shadow)
            log::error("You are getting a draw description for a shadowmap shader");
        
        DrawDescription description{};
        description.shader = shader;
        description.texture = texture;
        description.uniform = registerDrawUniforms<UBO...>(shader);
        description.geometry = registerGeometryBuffer(vertices, indices);
        
        updateDrawDescriptorSets(description);
        return description;
    }
    
    template <typename... UBO, typename V, typename U, typename I,
              std::enable_if_t<Reflection::is_reflectable<V> && Reflection::is_reflectable<U> && std::is_integral_v<I>, bool> = true>
    DrawDescription getDrawDescriptionI(const std::vector<V> &vertices, const std::vector<U> &instanced_data,
                                        const std::vector<I> &indices, ShaderID shader, TextureID texture = no_texture) {
        
        if (not api.shaders.at(shader).is_instanced)
            log::error("You are getting a draw description for a regular shader using the function for instanced rendering, use getDrawDescription()");
        
        DrawDescription description = getDrawDescription<UBO...>(vertices, indices, shader, texture, true);
        #if defined USE_VULKAN
        description.instance = registerInstancedBuffer(api, instanced_data);
        #elif defined USE_OPENGL
        description.instance = registerInstancedBuffer(api, vertices, instanced_data, api.geometry_buffer_data.at(description.geometry).vao);
        #endif
        
        return description;
    }
    
    template <typename UBO, std::enable_if_t<Reflection::is_reflectable<UBO>, bool> = true>
    struct GlobalUniforms {
        inline static std::vector<str> members{};
        inline static decltype(tuple_from_variant(Reflection::as_type_list<UBO>())) values{};
    };
    
    template <typename UBO, Str name, typename T>
    void setGlobalUniform(T t) {
        if (not std::count(GlobalUniforms<UBO>::members.begin(), GlobalUniforms<UBO>::members.end(), str(name.sv())))
            GlobalUniforms<UBO>::members.push_back(str(name.sv()));
        constexpr size_t index = Reflection::get_index_c<name, UBO>();
        std::get<index>(GlobalUniforms<UBO>::values) = t;
    }
    
    template <typename UBO, Str name>
    void unsetGlobalUniform() {
        auto it = std::find(GlobalUniforms<UBO>::members.begin(), GlobalUniforms<UBO>::members.end(), str(name.sv()));
        if (it != GlobalUniforms<UBO>::members.end())
            GlobalUniforms<UBO>::members.erase(it);
    }
    
    void draw_(DrawDescription &description);
    
    template <typename... UBO>
    void draw(DrawDescription &description, UBO& ...ubo) {
        draw_(description);
        
        ([&](){
        if constexpr (Reflection::is_reflectable<UBO>) {
            for (str member : GlobalUniforms<UBO>::members) {
                size_t index = Reflection::get_index<UBO>(member);
                for_<Reflection::as_type_list<UBO>>([&](auto i){
                    if (i.value == index) {
                        using M = std::variant_alternative_t<i.value, Reflection::as_type_list<UBO>>;
                        constexpr size_t offset = Reflection::get_offset_c<UBO, i.value>();
                        M* m = (M*)((size_t*)&ubo + offset/8);
                        *m = std::get<i.value>(GlobalUniforms<UBO>::values);
                    }
                });
            }
        }
        }(), ...);
        
        updateDrawUniformBuffer(description, ubo...);
    }

    TextureID getTextureID(str path, Channels ch = TEXTURE_CHANNELS_RGBA);

    void draw(DrawDescription &description, glm::mat4 model);
}
