# CGJ sample project - self-contained starter mirroring the course layout.
# Every dependency lives under libraries/ and is built from source with
# add_subdirectory, so the project compiles the same way on Linux, macOS and
# Windows without relying on system packages. The libraries/ folder is part of
# the project and must be submitted together with the source (see README.md).

cmake_minimum_required(VERSION 3.28)
project(sample LANGUAGES CXX)

# Language standard. CXX_EXTENSIONS OFF forces -std=c++20 rather than the GNU
# dialect (-std=gnu++20), keeping the code portable across compilers.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Emit build/compile_commands.json so clangd (Zed, VS Code, ...) knows the exact
# flags and include paths for each file. Without it clangd guesses and reports
# false errors such as "use of undeclared identifier 'std'".
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build every dependency as a static library folded into the final executable,
# so there are no .so/.dylib/.dll files to ship alongside the binary.
set(BUILD_SHARED_LIBS OFF)

# Trim GLFW's own build: we only want the library, not its docs/tests/examples.
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)

# Pull each dependency into the build.
# GLEW's CMake project lives in build/cmake; the second argument is the binary
# (build-output) directory CMake uses for it.
add_subdirectory(libraries/glew/build/cmake libraries/glew)
add_subdirectory(libraries/glm)
add_subdirectory(libraries/glfw)

# OpenGL itself is provided by the system/driver (Mesa on Linux, the OpenGL
# framework on macOS, opengl32 on Windows); it is found, not built.
find_package(OpenGL REQUIRED)

add_executable(sample src/main.cpp)

# GLEW's header includes the legacy GL/glu.h unless told otherwise. GLU is
# deprecated and unused by modern OpenGL, and it is not always present (e.g. the
# Nix dev shell), so skip it for a portable build.
target_compile_definitions(sample PRIVATE GLEW_NO_GLU)

# GLEW's headers are not installed, so point the target at the in-tree include
# dir. SYSTEM suppresses warnings coming from third-party headers.
target_include_directories(sample
  SYSTEM PRIVATE
    libraries/glew/include
)

# Link the static GLEW target (glew_s), plus GLFW, GLM (header-only) and the
# system OpenGL.
target_link_libraries(sample
  PRIVATE
    glm
    OpenGL::GL
    glew_s
    glfw
)
