//* types
//      common graphic libraries import definitions
#pragma once

//: glad vulkan loader
#include <glad/vulkan.h>

//: vulkan memory allocator (load funcions dinamically)
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

//: glfw windowing library
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//: spirv-cross, shader reflection and cross compilation
#include "spirv_glsl.hpp"
namespace spv_c = spirv_cross;

//: standard types
#include "std_types.h"