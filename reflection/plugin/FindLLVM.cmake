# FIND CLANG / LLVM
# ------------------------------------------------------------------------------
# Clang definitions (CLANG_DEFINITIONS)
function(set_clang_definitions config_cmd)
    execute_process(
        COMMAND ${config_cmd} --cppflags
        OUTPUT_VARIABLE llvm_cppflags
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCHALL "(-D[^ ]*)" dflags ${llvm_cppflags})
    string(REGEX MATCHALL "(-U[^ ]*)" uflags ${llvm_cppflags})
    list(APPEND cxxflags ${dflags})
    list(APPEND cxxflags ${uflags})

    set(CLANG_DEFINITIONS ${cxxflags} PARENT_SCOPE)
endfunction()

# Test if clang is installed (CLANG_INSTALLED)
function(is_clang_installed config_cmd)
    execute_process(
        COMMAND ${config_cmd} --includedir
        OUTPUT_VARIABLE include_dirs
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
        COMMAND ${config_cmd} --src-root
        OUTPUT_VARIABLE llvm_src_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(FIND ${include_dirs} ${llvm_src_dir} result)

    set(CLANG_INSTALLED ${result} PARENT_SCOPE)
endfunction()

# Clang include directories (CLANG_INCLUDE_DIRS)
function(set_clang_include_dirs config_cmd)
    is_clang_installed(${config_cmd})
    if(CLANG_INSTALLED)
        execute_process(
        COMMAND ${config_cmd} --includedir
        OUTPUT_VARIABLE include_dirs
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        execute_process(
        COMMAND ${config_cmd} --src-root
        OUTPUT_VARIABLE llvm_src_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(
        COMMAND ${config_cmd} --obj-root
        OUTPUT_VARIABLE llvm_obj_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE)
        list(APPEND include_dirs "${llvm_src_dir}/include")
        list(APPEND include_dirs "${llvm_obj_dir}/include")
        list(APPEND include_dirs "${llvm_src_dir}/tools/clang/include")
        list(APPEND include_dirs "${llvm_obj_dir}/tools/clang/include")
    endif()

    set(CLANG_INCLUDE_DIRS ${include_dirs} PARENT_SCOPE)
endfunction()

# Find llvm-config
find_program(LLVM_CONFIG NAMES llvm-config llvm-config-17 llvm-config-16 llvm-config-15 llvm-config-14 llvm-config-13 llvm-config-12 llvm-config-11 llvm-config-10 llvm-config-9 llvm-config-8 llvm-config-64 PATHS ENV LLVM_PATH)
if(LLVM_CONFIG)
    message(STATUS "[INFO] Found llvm-config: ${LLVM_CONFIG}")
else()
    message(WARNING "[WARN] Can't find llvm-config - LLVM is required to build the reflection plugin")
    return()
endif()

# LLVM Cmake directory
execute_process(
    COMMAND ${LLVM_CONFIG} --cmakedir
    OUTPUT_VARIABLE LLVM_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Set clang definitions
set_clang_definitions(${LLVM_CONFIG})
set_clang_include_dirs(${LLVM_CONFIG})

set(CLANG_FOUND 1)
# ------------------------------------------------------------------------------