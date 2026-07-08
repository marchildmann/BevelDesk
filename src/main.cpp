// Windows 95 desktop in Dear ImGui — application entry point.
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "app/app.h"
#include "platform/gl.h"
#include "platform/platform.h"
#include "shell/desktop.h"
#include "shell/dosprompt.h"
#include "shell/explorer.h"
#include "shell/shutdown.h"
#include "shell/taskbar.h"
#include "theme95/theme95.h"

AppState g_app;

static void GlfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

int main(int argc, char** argv) {
    bool screenshot_mode = false;
    bool fullscreen = false;
    bool borderless = false;
    int win_w = 1024, win_h = 768;
    const char* screenshot_path = "screenshot.png";
    const char* demo = nullptr; // "start" | "max" | "nav" | "icons" | "shutdown" | "dos" | "sysmenu"
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--screenshot") == 0) {
            screenshot_mode = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') screenshot_path = argv[++i];
        } else if (std::strcmp(argv[i], "--demo") == 0 && i + 1 < argc) {
            demo = argv[++i];
        } else if (std::strcmp(argv[i], "--windowed") == 0 ||
                   std::strcmp(argv[i], "--size") == 0) {
            int w = 0, h = 0;
            if (i + 1 < argc && std::sscanf(argv[i + 1], "%dx%d", &w, &h) == 2) {
                ++i;
                if (w >= 640 && h >= 480) { win_w = w; win_h = h; }
            }
        } else if (std::strcmp(argv[i], "--borderless") == 0) {
            borderless = true;   // square corners: no macOS title bar / rounding
        } else if (std::strcmp(argv[i], "--fullscreen") == 0) {
            fullscreen = true;   // total immersion: no host chrome at all
        }
    }

    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#ifdef __APPLE__
    // Screenshot QA stays 1:1 (deterministic 1024x768 PNG); interactive mode
    // renders at native Retina resolution for crisp text.
    if (screenshot_mode) glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    // Default: a normal 1024x768 window. --fullscreen for total immersion
    // ("booting into Windows 95"), --borderless for a square-cornered
    // undecorated window, --windowed WxH to pick a size.
    GLFWwindow* window = nullptr;
    if (screenshot_mode) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = glfwCreateWindow(win_w, win_h, "BevelDesk", nullptr, nullptr);
    } else if (fullscreen) {
        GLFWmonitor* mon = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(mon);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        window = glfwCreateWindow(mode->width, mode->height, "BevelDesk", mon, nullptr);
    } else {
        if (borderless) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        window = glfwCreateWindow(win_w, win_h, "BevelDesk", nullptr, nullptr);
    }
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(screenshot_mode ? 0 : 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;                       // deterministic layout
    io.ConfigWindowsMoveFromTitleBarOnly = true;    // we drag via our own caption
    t95::ApplyStyle();
    float cscale_x = 1.0f, cscale_y = 1.0f;
    glfwGetWindowContentScale(window, &cscale_x, &cscale_y);
    LoadFonts(io, screenshot_mode ? 1.0f : (cscale_x > cscale_y ? cscale_x : cscale_y));

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
    g_app.checker_tex = CreateCheckerTexture();

    bool demo_dos = screenshot_mode && demo && std::strcmp(demo, "dos") == 0;
    if (screenshot_mode) {
        // demo layout: two Explorer windows (verifies inactive vs active caption)
        g_app.OpenExplorer("/");
        const char* home = std::getenv("HOME");
        g_app.OpenExplorer(home ? home : "/");
        if (demo_dos) OpenDosPrompt(g_app);
        if (demo && std::strcmp(demo, "start") == 0) g_app.force_start_open = true;
        if (demo && std::strcmp(demo, "max") == 0) g_app.wins.back().maximized = true;
        if (demo && std::strcmp(demo, "icons") == 0) g_app.wins.back().view_mode = 1;
        if (demo && std::strcmp(demo, "shutdown") == 0) {
            g_app.shutdown_open = true;
            g_app.shutdown_opened_this_frame = true;
        }
    }

    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --demo sysmenu: pop the system menu on the focused explorer
        if (screenshot_mode && demo && std::strcmp(demo, "sysmenu") == 0 && frame == 3 &&
            !g_app.wins.empty())
            t95::DebugOpenSysMenu(g_app.wins.back().imgui_id.c_str());

        // --demo nav: exercise navigation + minimize logic under the screenshot QA loop
        if (screenshot_mode && demo && std::strcmp(demo, "nav") == 0 && frame == 3 &&
            g_app.wins.size() >= 2) {
            g_app.wins[0].Navigate("/usr");
            g_app.wins[1].minimized = true;
            g_app.wins[0].request_focus = true;
            g_app.active_win_id = g_app.wins[0].id;
        }

        // --demo dos: type a command into the live shell mid-run
        if (demo_dos && frame == 40 && !g_app.dos_wins.empty()) {
            const char* cmd = "echo; echo '*** real zsh inside Windows 95 ***'; ls\r";
            g_app.dos_wins.back()->pty.Write(cmd, std::strlen(cmd));
        }

        DrawDesktop(g_app);
        DrawExplorers(g_app);
        DrawDosPrompts(g_app);
        DrawTaskbarAndStartMenu(g_app);
        DrawShutdownDialog(g_app);

        // Cmd+Q (macOS) / Ctrl+Q exits — fullscreen has no host chrome.
        // Suppressed while a DOS prompt has focus (Ctrl+Q is XON there).
        bool dos_focused = false;
        for (auto& d : g_app.dos_wins)
            if (d->focused && !d->minimized) dos_focused = true;
        if (!dos_focused && (io.KeySuper || io.KeyCtrl) &&
            ImGui::IsKeyPressed(ImGuiKey_Q, false))
            g_app.quit_requested = true;
        // Cmd+M minimizes the host window to the Dock (borderless has no button)
        if (io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_M, false))
            glfwIconifyWindow(window);
        if (g_app.quit_requested) glfwSetWindowShouldClose(window, GLFW_TRUE);

        ImGui::Render();
        int fw, fh;
        glfwGetFramebufferSize(window, &fw, &fh);
        glViewport(0, 0, fw, fh);
        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // the dos demo needs real time for the shell to start and print
        int shot_frame = demo_dos ? 140 : 9;
        if (screenshot_mode && frame == shot_frame) {
            SaveScreenshot(window, screenshot_path);
            glfwSwapBuffers(window);
            break;
        }
        glfwSwapBuffers(window);
        if (demo_dos) std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ++frame;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
