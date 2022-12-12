# CLANG PLUGIN
# ------------------------------------------------------------------------------
# Find Clang
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(LLVM)

if (CLANG_FOUND) # If the clang libraries are found, build the reflection plugin
    # LLVM CMake modules
    list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
    include(AddLLVM)
    include(HandleLLVMOptions)

    # Create the reflection plugin
    add_library(reflection MODULE reflection_plugin.cpp)

    # Link the plugin against the Clang libraries
    target_link_libraries(reflection ${CLANG_LIBS})

    # Include the Clang and LLVM headers
    target_include_directories(reflection PRIVATE ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})

    # Get plugin location
    message(STATUS "[INFO] Reflection plugin created")
else() # If there are no clang libraries, try to find the pre-built plugin
    find_library(REFLECTION_PLUGIN reflection PATHS ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/bin ${CMAKE_BINARY_DIR}/fresa/reflection/plugin ${CMAKE_CURRENT_SOURCE_DIR})
    
    if (REFLECTION_PLUGIN) # If the plugin is found, link it to the executable
        message(STATUS "[INFO] Found prebuild reflection plugin at ${REFLECTION_PLUGIN}")
    else() # If the plugin is not found, print a warning
        message(FATAL_ERROR "[ERROR] There are no clang libraries or prebuilt reflection plugin. Please install clang or build the plugin")
    endif()
endif()

# ------------------------------------------------------------------------------