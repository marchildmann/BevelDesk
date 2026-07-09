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
#ifdef __EMSCRIPTEN__
  #include <emscripten.h>
#endif

#include "app/app.h"
#include "platform/gl.h"
#include "platform/platform.h"
#include "shell/about.h"
#include "shell/desktop.h"
#include "shell/displayprops.h"
#include "shell/dosprompt.h"
#include "shell/drawer.h"
#include "shell/run.h"
#include "shell/explorer.h"
#include "shell/shutdown.h"
#include "shell/taskbar.h"
#include "theme95/theme95.h"

AppState g_app;

// Per-frame state shared between the native while-loop and Emscripten's
// browser-driven main-loop callback.
struct FrameCtx {
    GLFWwindow* window = nullptr;
    bool screenshot_mode = false;
    const char* demo = nullptr;
    bool demo_dos = false;
    int win_w = 1024, win_h = 768;
    const char* screenshot_path = "screenshot.png";
    int frame = 0;
    bool stop = false;
};

static float PixelRatio(GLFWwindow* window) {
    int ww = 1, wh = 1, fw = 1, fh = 1;
    glfwGetWindowSize(window, &ww, &wh);
    glfwGetFramebufferSize(window, &fw, &fh);
    return (ww > 0) ? (float)fw / ww : 1.0f;
}

static void GlfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

// One rendered frame. Called directly by the native loop, and by the browser
// via emscripten_set_main_loop_arg on the web.
static void MainLoopFrame(void* arg) {
    FrameCtx& ctx = *(FrameCtx*)arg;
    GLFWwindow* window = ctx.window;
    ImGuiIO& io = ImGui::GetIO();

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    // apply zoom: shrink the logical display so fixed-pixel UI is magnified,
    // and map the cursor back into logical space (must run before NewFrame).
    if (g_app.zoom != 1.0f) {
        float Z = g_app.zoom, R = ctx.screenshot_mode ? 1.0f : PixelRatio(window);
        int ww, wh;
        glfwGetWindowSize(window, &ww, &wh);
        if (ctx.screenshot_mode) { ww = ctx.win_w; wh = ctx.win_h; }
        io.DisplaySize = ImVec2(ww / Z, wh / Z);
        io.DisplayFramebufferScale = ImVec2(R * Z, R * Z);
        // 1.92 input is event-based: the GLFW backend already queued the raw
        // cursor position, and NewFrame would overwrite any io.MousePos we
        // set. Queue a corrected event *after* the backend's so it wins.
        if (!ctx.screenshot_mode && glfwGetWindowAttrib(window, GLFW_HOVERED)) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            io.AddMousePosEvent((float)(mx / Z), (float)(my / Z));
        }
    }

    ImGui::NewFrame();

    // ---- zoom input: Ctrl/Cmd + mouse-wheel (layout-independent) or +/-/0.
    // Handled here, before the UI draws, so a modifier+wheel zooms instead
    // of scrolling the list under the cursor.
    {
        static const float steps[] = { 1.0f, 1.25f, 1.5f, 2.0f, 2.5f, 3.0f };
        const int nz = (int)(sizeof(steps) / sizeof(steps[0]));
        bool mod = io.KeyCtrl || io.KeySuper;
        bool dos_focus = g_app.drawer_focused;   // don't steal Ctrl-combos from a shell
        for (auto& d : g_app.dos_wins)
            if (d->focused && !d->minimized) dos_focus = true;
        int cur = 0;
        for (int k = 0; k < nz; ++k) if (steps[k] <= g_app.zoom + 0.01f) cur = k;
        int dir = 0;
        if (mod && io.MouseWheel > 0.1f) dir = 1;
        else if (mod && io.MouseWheel < -0.1f) dir = -1;
        if (mod && io.MouseWheel != 0.0f) io.MouseWheel = 0.0f; // don't scroll lists
        if (mod && !dos_focus) {
            if (ImGui::IsKeyPressed(ImGuiKey_Equal, false) ||
                ImGui::IsKeyPressed(ImGuiKey_KeypadAdd, false)) dir = 1;
            else if (ImGui::IsKeyPressed(ImGuiKey_Minus, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract, false)) dir = -1;
            else if (ImGui::IsKeyPressed(ImGuiKey_0, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_Keypad0, false)) {
                g_app.zoom = 1.0f;
            }
        }
        if (dir > 0 && cur < nz - 1) g_app.zoom = steps[cur + 1];
        else if (dir < 0 && cur > 0) g_app.zoom = steps[cur - 1];
    }

    // Toggle the terminal drawer: F12 (layout-independent) or Ctrl+` (US kbd).
    // ImGuiKey_GraveAccent is the physical US-backtick key, which differs on
    // non-US layouts — F12 always works.
    if (ImGui::IsKeyPressed(ImGuiKey_F12, false) ||
        (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false)))
        ToggleDrawer(g_app);

    // --demo sysmenu: pop the system menu on the focused explorer
    if (ctx.screenshot_mode && ctx.demo && std::strcmp(ctx.demo, "sysmenu") == 0 &&
        ctx.frame == 3 && !g_app.wins.empty())
        t95::DebugOpenSysMenu(g_app.wins.back().imgui_id.c_str());

    // --demo nav: exercise navigation + minimize logic under the screenshot QA loop
    if (ctx.screenshot_mode && ctx.demo && std::strcmp(ctx.demo, "nav") == 0 &&
        ctx.frame == 3 && g_app.wins.size() >= 2) {
        g_app.wins[0].Navigate("/usr");
        g_app.wins[1].minimized = true;
        g_app.wins[0].request_focus = true;
        g_app.active_win_id = g_app.wins[0].id;
    }

    // --demo dos: type a command into the live shell mid-run
    if (ctx.demo_dos && ctx.frame == 40 && !g_app.dos_wins.empty()) {
        const char* cmd = "echo; echo '*** real zsh inside Windows 95 ***'; ls\r";
        g_app.dos_wins.back()->pty.Write(cmd, std::strlen(cmd));
    }

    DrawDesktop(g_app);
    DrawExplorers(g_app);
    DrawDosPrompts(g_app);
    DrawTaskbarAndStartMenu(g_app);
    // terminal drawer fills the strip below the (raised) taskbar
    {
        ImGuiViewport* mvp = ImGui::GetMainViewport();
        DrawTerminalDrawer(g_app, mvp->Pos.y + mvp->Size.y - g_app.drawer_height);
    }
    DrawDisplayProperties(g_app);
    DrawAbout(g_app);
    DrawRun(g_app);
    DrawShutdownDialog(g_app);   // last: its fade covers everything

    // Cmd+Q (macOS) / Ctrl+Q exits — no effect in the browser. Suppressed while
    // a shell (DOS window or the drawer) has focus, so Ctrl+Q reaches it.
    bool dos_focused = g_app.drawer_focused;
    for (auto& d : g_app.dos_wins)
        if (d->focused && !d->minimized) dos_focused = true;
#ifndef __EMSCRIPTEN__
    if (!dos_focused && (io.KeySuper || io.KeyCtrl) &&
        ImGui::IsKeyPressed(ImGuiKey_Q, false))
        g_app.quit_requested = true;
    if (io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_M, false))
        glfwIconifyWindow(window);
#endif
    if (g_app.quit_requested) ctx.stop = true;

    ImGui::Render();
    int fw, fh;
    glfwGetFramebufferSize(window, &fw, &fh);
    glViewport(0, 0, fw, fh);
    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // the dos demo needs real time for the shell to start and print
    int shot_frame = ctx.demo_dos ? 140 : 9;
    if (ctx.screenshot_mode && ctx.frame == shot_frame) {
        SaveScreenshot(window, ctx.screenshot_path);
        glfwSwapBuffers(window);
        ctx.stop = true;
        return;
    }
    glfwSwapBuffers(window);
#ifndef __EMSCRIPTEN__
    if (ctx.demo_dos) std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
    ++ctx.frame;
}

int main(int argc, char** argv) {
    bool screenshot_mode = false;
    bool fullscreen = false;
    bool borderless = false;
    bool night = false;
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
        } else if (std::strcmp(argv[i], "--night") == 0) {
            night = true;        // preview the NeXT Night scheme (Phase 3 tuning)
        } else if (std::strcmp(argv[i], "--zoom") == 0 && i + 1 < argc) {
            g_app.zoom = (float)std::atof(argv[++i]);    // startup zoom (QA/pref)
            if (g_app.zoom < 1.0f) g_app.zoom = 1.0f;
            if (g_app.zoom > 3.0f) g_app.zoom = 3.0f;
        }
    }

    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) return 1;

#ifdef __EMSCRIPTEN__
    // WebGL2 (~OpenGL ES 3.0); the GLSL is "#version 300 es".
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
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
#ifdef __EMSCRIPTEN__
    // In the browser the window maps to the page canvas; size comes from JS.
    (void)fullscreen; (void)borderless;
    window = glfwCreateWindow(win_w, win_h, "BevelDesk", nullptr, nullptr);
#else
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
#endif
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(screenshot_mode ? 0 : 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;                       // deterministic layout
    io.ConfigWindowsMoveFromTitleBarOnly = true;    // we drag via our own caption
    t95::ApplyStyle();
    // framebuffer/window ratio (2 on Retina). On 1.92, dynamic fonts rebake at
    // whatever density DisplayFramebufferScale implies, so zoom stays crisp
    // with no manual atlas rebuild.
    LoadFonts(io, screenshot_mode ? 1.0f : PixelRatio(window));
    if (night) {
        t95::ApplyScheme(t95::PaletteNight, t95::Style{true});
        g_app.desktop_color = t95::DESKTOP;   // dark desktop to match
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    // Wire GLFW to the page canvas so resizes flow into io.DisplaySize;
    // without this glfwGetFramebufferSize is 0 and nothing renders.
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);
    g_app.checker_tex = CreateCheckerTexture();

    // slow demos (dos, drawer) need real wall-clock for the shell to print
    bool demo_dos = screenshot_mode && demo &&
                    (std::strcmp(demo, "dos") == 0 || std::strcmp(demo, "drawer") == 0);
    if (screenshot_mode) {
        // demo layout: two Explorer windows (verifies inactive vs active caption)
        g_app.OpenExplorer("/");
        const char* home = std::getenv("HOME");
        g_app.OpenExplorer(home ? home : "/");
        if (demo && std::strcmp(demo, "dos") == 0) OpenDosPrompt(g_app);
        if (demo && std::strcmp(demo, "drawer") == 0) ToggleDrawer(g_app);
        if (demo && std::strcmp(demo, "start") == 0) g_app.force_start_open = true;
        if (demo && std::strcmp(demo, "max") == 0) g_app.wins.back().maximized = true;
        if (demo && std::strcmp(demo, "icons") == 0) g_app.wins.back().view_mode = 1;
        if (demo && std::strcmp(demo, "shutdown") == 0) {
            g_app.shutdown_open = true;
            g_app.shutdown_opened_this_frame = true;
        }
        if (demo && std::strcmp(demo, "display") == 0) OpenDisplayProperties(g_app);
        if (demo && std::strcmp(demo, "about") == 0) OpenAbout(g_app);
        if (demo && std::strcmp(demo, "run") == 0) OpenRun(g_app);
    }
#ifdef __EMSCRIPTEN__
    // Greet the browser visitor with the mock filesystem already open.
    g_app.OpenExplorer("/");
#endif

    FrameCtx ctx;
    ctx.window = window;
    ctx.screenshot_mode = screenshot_mode;
    ctx.demo = demo;
    ctx.demo_dos = demo_dos;
    ctx.win_w = win_w;
    ctx.win_h = win_h;
    ctx.screenshot_path = screenshot_path;

#ifdef __EMSCRIPTEN__
    // The browser owns frame timing: hand it our callback (fps=0 → rAF).
    emscripten_set_main_loop_arg(MainLoopFrame, &ctx, 0, true);
#else
    while (!ctx.stop && !glfwWindowShouldClose(window))
        MainLoopFrame(&ctx);
#endif

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
