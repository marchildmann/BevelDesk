cmake_minimum_required(VERSION 3.16)
project(beveldesk LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glfw
  URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
)
FetchContent_Declare(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.tar.gz
)
FetchContent_Declare(stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw imgui stb)

add_library(imgui_lib STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC glfw)

add_executable(beveldesk
  src/main.cpp
  src/app/app.cpp
  src/app/term.cpp
  src/theme95/bevel.cpp
  src/theme95/widgets.cpp
  src/theme95/window.cpp
  src/theme95/style.cpp
  src/theme95/icons95.cpp
  src/shell/desktop.cpp
  src/shell/taskbar.cpp
  src/shell/explorer.cpp
  src/shell/shutdown.cpp
  src/shell/dosprompt.cpp
  src/shell/displayprops.cpp
  src/shell/about.cpp
  src/shell/run.cpp
  src/platform/platform.cpp
  src/platform/pty.cpp
  src/platform/stb_impl.cpp
)
target_include_directories(beveldesk PRIVATE src ${stb_SOURCE_DIR})
target_link_libraries(beveldesk PRIVATE imgui_lib)

if(APPLE)
  target_compile_definitions(beveldesk PRIVATE GL_SILENCE_DEPRECATION)
  target_link_libraries(beveldesk PRIVATE "-framework OpenGL")
  # Assemble a double-clickable .app bundle (Finder launch, no Terminal)
  set(APP_CONTENTS "${CMAKE_BINARY_DIR}/BevelDesk.app/Contents")
  add_custom_command(TARGET beveldesk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_CONTENTS}/MacOS"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:beveldesk>" "${APP_CONTENTS}/MacOS/beveldesk"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/macos/Info.plist" "${APP_CONTENTS}/Info.plist"
    COMMENT "Bundling BevelDesk.app")
elseif(WIN32)
  target_link_libraries(beveldesk PRIVATE opengl32)
else()
  find_package(OpenGL REQUIRED)
  target_link_libraries(beveldesk PRIVATE OpenGL::GL util) # util: forkpty on glibc
endif()
