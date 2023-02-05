# building the project

## using a template

This projects provides a ready to use [template](https://github.com/josekoalas/mermelada) with all required dependencies preconfigured. It can be used in two ways, either by creating your own copy _(recommended)_ or by cloning it directly.

**a. creating a copy**

Go to [josekoalas/mermelada](https://github.com/josekoalas/mermelada) and click on the green button that says "Use this template". **This requires a GitHub account**. You will be taken to a new screen to configure the new repository settings. Set a name and the options you prefer and click on "Create repository from template". Now you will have a copy of the template in your profile ready to use. Download it with the following command, replacing `your_github_username` and `repository_name` with the appropiate values.

```sh
git clone --recurse-submodules https://github.com/your_github_username/repository_name
cd repository_name
```

**b. cloning the template**

If you don't have a github account or you don't want to create a copy, you can download the template directly by doing:

```sh
git clone --recurse-submodules https://github.com/josekoalas/mermelada
cd mermelada
```

## building the project

The template repository already includes a `CMakeLists.txt` with all the configuration needed. Please ensure you have a [compatible compiler](#tested-compilers) and a recent version of [cmake](#extra-installing-cmake) and the [vulkan sdk](#extra-installing-the-vulkan-sdk) before proceeding. To configure and build the project use the following two commands respectively:

```sh
cmake -S . -B build
cmake --build build
```

## tested compilers

These compilers have been tested to work. Older versions might work but are not officially supported.

- clang 14.0+

MSVC and gcc may work, but they are out of the scope of the project since the reflection system is a clang plugin, so it is not possible to use it with other compilers.

On linux systems glfw requires some [development libraries](https://www.glfw.org/docs/latest/compile_guide.html#compile_deps_x11). For X11 and debian you should be fine using `sudo apt install xorg-dev`, for other systems or window managers check the complete list.

## extra: installing compilers

This is a very brief and general guide to install the most recent compilers. Your system requirements might vary so please refer to the original instructions when in doubt.

**debian/ubuntu:**

```sh
# gcc
apt install build-essential
# clang
apt install clang lldb lld
```

**macos:**

If you install [XCode](https://developer.apple.com/xcode) or the [Command Line Tools](https://developer.apple.com/download/all) it will come with Apple clang. To get the version 14.0, which includes improved support for `c++20`, you need to download the beta of XCode 14. Regular clang and gcc can be installed with [homebrew](https://brew.sh):

```sh
# gcc
brew install gcc
# clang
brew install llvm
```

**windows:**

You can use clang with [Visual Studio](https://docs.microsoft.com/en-us/cpp/build/clang-support-msbuild?view=msvc-170) or install gcc with [MinGW](https://sourceforge.net/projects/mingw/files/Installer/mingw-get-setup.exe/download). As mentioned above, MSVC is a work in progress.

## extra: installing cmake

`CMake` is our build system of choice. You can download it from the [official website](https://cmake.org/download) or, if you are using an unix like system, get it from a package manager:

**debian/ubuntu:**

```sh
apt install cmake
# or
snap install cmake
```

**macos:**

```sh
brew install cmake
```

## extra: installing the vulkan sdk

`Vulkan` is the graphics API we use to render our scenes. It is a very powerful and customizable API that is used by many applications. To develop with Vulkan you need to install the latest version of the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home). There are readily available binaries for all platforms that take care of the installation.


## advanced: manual configuration

You can ignore the template and add **fresa** directly into your project. The main library is located in [josekoalas/fresa](https://github.com/josekoalas/fresa). Please note when adding includes that include directories are specified without the folder, so you have to specify all include files, not just recursive directories. Also make sure that all [required libraries](#advanced-required-libraries) are linked propperly.

To use **fresa** all you need is to include "engine.h" and call `fresa::run`. Configuration parameters are a work in progress.

## advanced: required libraries

- [**fmt**](https://github.com/fmtlib/fmt) _[MIT]_: Used for general string formatting, mainly as a basis to the logging system (`log.h`). May be replaced with `std::format` in the future, but it depends on its capabilities.

**standard libraries**

Some compilers don't have full support of all `c++20` features. Therefore, while those compilers get support, you can alternatively use this libraries that the standard is based on as a stand in. Once compiler support is widespread these will be removed.

- [**range-v3**](https://github.com/ericniebler/range-v3) _[BSL 1.0]_: For `std::ranges`.
- [**jthread**](https://github.com/josuttis/jthread) _[CC BY 4.0]_: For `std::jthread`.

**rendering libraries**

Despite incorporating its own renderer, some low level functions are required to create and manage a cross platform window, surface and input. For this purpose we use glfw3. We also use the vulkan memory allocator for managing memory inside the vulkan application and SPIRV-Cross for shader reflection.

- [**glfw**](https://github.com/glfw/glfw) _[ZLIB]_: For window creation and input.
- [**vma**](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) _[MIT]_: For vulkan memory management.
- [**spirv-cross**](https://github.com/KhronosGroup/SPIRV-Cross) _[Apache 2.0]_: For converting spirv to glsl and reflecting shaders.