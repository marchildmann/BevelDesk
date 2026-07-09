#include "shell/taskbar.h"
#include "shell/displayprops.h"
#include "shell/dosprompt.h"
#include "shell/drawer.h"
#include "shell/about.h"
#include "shell/run.h"
#include <cstdlib>
#include "app/app.h"
#include "theme95/theme95.h"
#include "theme95/icons95.h"
#include "imgui_internal.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

using namespace t95;

// ---- start menu -------------------------------------------------------------

// Draw text rotated 90° CCW (reads bottom-to-top, like the Win95 sidebar).
// ImDrawList can't rotate text, but we can rotate the glyph vertices after
// submission: pivot = the bottom-left of the vertical text run.
static void AddTextBottomUp(ImDrawList* dl, ImFont* font, ImVec2 pivot,
                            ImU32 col, const char* text) {
    int vtx_start = dl->VtxBuffer.Size;
    dl->AddText(font, 0.0f, pivot, col, text);
    for (int i = vtx_start; i < dl->VtxBuffer.Size; ++i) {
        ImDrawVert& v = dl->VtxBuffer.Data[i];
        float dx = v.pos.x - pivot.x, dy = v.pos.y - pivot.y;
        v.pos = ImVec2(pivot.x + dy, pivot.y - dx);
    }
}

static void DrawStartMenu(AppState& app, float taskbar_top) {
    if (!app.start_open) return;

    struct Item { const char* label; bool arrow; bool separator_before; };
    static const Item items[] = {
        { "Programs", false, false },
        { "Documents", false, false },
        { "Settings", false, false },
        { "Find", false, false },
        { "Help", false, false },
        { "MS-DOS Prompt", false, false },
        { "Run...", false, false },
        { "Shut Down...", false, true },
    };
    const float item_h = 30, side_w = 22, width = 176;
    float height = 6; // borders
    for (const Item& it : items) height += item_h + (it.separator_before ? 7 : 0);

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 pos(vp->Pos.x + 2, taskbar_top - height);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(ImVec2(width, height));
    if (app.start_opened_this_frame) ImGui::SetNextWindowFocus();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(FACE));
    ImGui::Begin("##startmenu", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoSavedSettings);
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mn = ImGui::GetWindowPos();
    ImVec2 mx(mn.x + width, mn.y + height);
    BevelRaised(dl, mn, mx);

    // navy sidebar band with vertical brand text, like the original's "Windows95"
    ImVec2 side_mn(mn.x + 3, mn.y + 3), side_mx(mn.x + 3 + side_w, mx.y - 3);
    dl->AddRectFilled(side_mn, side_mx, NAVY);
    ImFont* brand_font = FontBold ? FontBold : ImGui::GetFont();
    float fh = ImGui::GetFontSize();   // 1.92: ImFont has no fixed FontSize field
    AddTextBottomUp(dl, brand_font,
                    ImVec2(side_mn.x + (side_w - fh) * 0.5f, side_mx.y - 6),
                    TITLE_TEXT, "BevelDesk");

    float y = mn.y + 3;
    for (const Item& it : items) {
        if (it.separator_before) {
            ThinSunken(dl, ImVec2(side_mx.x + 4, y + 3), ImVec2(mx.x - 6, y + 5));
            y += 7;
        }
        ImVec2 imin(side_mx.x + 1, y), imax(mx.x - 3, y + item_h);
        ImGui::SetCursorScreenPos(imin);
        ImGui::PushID(it.label);
        bool clicked = ImGui::InvisibleButton("##item", ImVec2(imax.x - imin.x, item_h));
        ImGui::PopID();
        bool hov = ImGui::IsItemHovered();
        if (hov) dl->AddRectFilled(imin, imax, SEL);
        ImU32 tcol = hov ? SEL_TEXT : TEXT;
        dl->AddText(ImVec2(imin.x + 10, y + (item_h - ImGui::GetFontSize()) * 0.5f), tcol, it.label);
        if (it.arrow) {
            float ax = imax.x - 14, ay = y + item_h * 0.5f;
            dl->AddTriangleFilled(ImVec2(ax, ay - 4), ImVec2(ax + 5, ay), ImVec2(ax, ay + 4), tcol);
        }
        if (clicked) {
            std::string label = it.label;
            const char* home = std::getenv("HOME");
            if (label == "Shut Down...") {
                app.shutdown_open = true;
                app.shutdown_opened_this_frame = true;
            } else if (label == "MS-DOS Prompt" || label == "Programs") {
                OpenDosPrompt(app);
            } else if (label == "Settings") {
                OpenDisplayProperties(app);
            } else if (label == "Documents") {
                app.OpenExplorer(home ? home : "/");
            } else if (label == "Find") {
                app.OpenExplorer("/");
            } else if (label == "Help") {
                OpenAbout(app);
            } else if (label == "Run...") {
                OpenRun(app);
            }
            app.start_open = false;
        }
        y += item_h;
    }

    // click elsewhere -> close (menu had focus while open)
    if (!app.start_opened_this_frame && !ImGui::IsWindowFocused())
        app.start_open = false;

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// ---- taskbar ----------------------------------------------------------------

void DrawTaskbarAndStartMenu(AppState& app) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    // when the terminal drawer is open the taskbar rides up above it, becoming
    // the drawer's handle/lid; the shell fills the strip below.
    float top = vp->Pos.y + vp->Size.y - TASKBAR_H;
    if (app.drawer_open) top -= app.drawer_height;
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, top));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, TASKBAR_H));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(FACE));
    ImGui::Begin("##taskbar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mn = ImGui::GetWindowPos();
    ImVec2 mx(mn.x + vp->Size.x, mn.y + TASKBAR_H);

    dl->AddRectFilled(mn, mx, FACE);
    dl->AddRectFilled(ImVec2(mn.x, mn.y), ImVec2(mx.x, mn.y + 1), HILIGHT); // top edge

    // when the drawer is open, the taskbar's top edge is a resize splitter
    if (app.drawer_open) {
        ImGui::SetCursorScreenPos(ImVec2(mn.x, mn.y - 2));
        ImGui::InvisibleButton("##drawer_resize", ImVec2(mx.x - mn.x, 5));
        if (ImGui::IsItemHovered() || ImGui::IsItemActive())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        if (ImGui::IsItemActive()) {
            app.drawer_height -= ImGui::GetIO().MouseDelta.y;   // drag up = taller
            float maxh = vp->Size.y - TASKBAR_H - 80;
            if (app.drawer_height < 80) app.drawer_height = 80;
            if (app.drawer_height > maxh) app.drawer_height = maxh;
        }
        // centered grip
        float gx = (mn.x + mx.x) * 0.5f;
        dl->AddRectFilled(ImVec2(gx - 16, mn.y + 2), ImVec2(gx + 16, mn.y + 3), SHADOW);
    }

    const float btn_y = mn.y + 4, btn_h = 22;

    // ---- Start button ----
    ImVec2 sb_min(mn.x + 2, btn_y), sb_max(sb_min.x + 64, btn_y + btn_h);
    ImGui::SetCursorScreenPos(sb_min);
    bool start_clicked = ImGui::InvisibleButton("##start", ImVec2(64, btn_h));
    bool start_held = ImGui::IsItemActive() && ImGui::IsItemHovered();
    bool start_down = start_held || app.start_open;
    if (start_down) BevelPressed(dl, sb_min, sb_max);
    else            BevelRaised(dl, sb_min, sb_max);
    float so = start_down ? 1.0f : 0.0f;
    icons95::BevelDeskLogo(dl, ImVec2(sb_min.x + 4 + so, sb_min.y + 4 + so));
    float sty = sb_min.y + (btn_h - ImGui::GetFontSize()) * 0.5f;
    AddTextBold(dl, ImVec2(sb_min.x + 23 + so, sty + so), TEXT, "Start");

    app.start_opened_this_frame = false;
    if (app.force_start_open) {
        app.start_open = true;
        app.start_opened_this_frame = true;
    }
    if (start_clicked) {
        app.start_open = !app.start_open;
        app.start_opened_this_frame = app.start_open;
    }

    // ---- terminal-drawer toggle, right of Start ----
    ImVec2 tb_min(sb_max.x + 4, btn_y), tb_max(tb_min.x + 26, btn_y + btn_h);
    ImGui::SetCursorScreenPos(tb_min);
    bool term_clicked = ImGui::InvisibleButton("##termtoggle", ImVec2(26, btn_h));
    bool term_held = ImGui::IsItemActive() && ImGui::IsItemHovered();
    bool term_down = term_held || app.drawer_open;
    if (term_down) BevelPressed(dl, tb_min, tb_max);
    else           BevelRaised(dl, tb_min, tb_max);
    float to = term_down ? 1.0f : 0.0f;
    icons95::MiniDos14(dl, ImVec2(tb_min.x + 6 + to, tb_min.y + 4 + to));
    if (term_clicked) ToggleDrawer(app);

    // ---- clock well (far right) ----
    const float clock_w = 72;
    ImVec2 ck_min(mx.x - 4 - clock_w, btn_y), ck_max(mx.x - 4, btn_y + btn_h);
    ThinSunken(dl, ck_min, ck_max);
    {
        time_t t = time(nullptr);
        struct tm* lt = localtime(&t);
        char buf[16];
        strftime(buf, sizeof(buf), "%I:%M %p", lt);
        const char* txt = (buf[0] == '0') ? buf + 1 : buf;   // "3:07 PM" not "03:07 PM"
        ImVec2 ts = ImGui::CalcTextSize(txt);
        dl->AddText(ImVec2(ck_min.x + (clock_w - ts.x) * 0.5f,
                           ck_min.y + (btn_h - ts.y) * 0.5f), TEXT, txt);
    }

    // ---- task buttons (Explorer + MS-DOS windows) ----
    struct TaskEntry {
        int id;
        const char* title;
        bool* minimized;
        bool* focused;
        int* request_focus;
        void (*icon)(ImDrawList*, ImVec2);
    };
    std::vector<TaskEntry> entries;
    entries.reserve(app.wins.size() + app.dos_wins.size());
    for (auto& w : app.wins)
        entries.push_back({ w.id, w.title.c_str(), &w.minimized, &w.focused,
                            &w.request_focus, icons95::MiniComputer14 });
    for (auto& d : app.dos_wins)
        entries.push_back({ d->id, d->title.c_str(), &d->minimized, &d->focused,
                            &d->request_focus, icons95::MiniDos14 });

    float tx = tb_max.x + 8;   // start task buttons past the drawer toggle
    float avail = ck_min.x - 8 - tx;
    int n = (int)entries.size();
    if (n > 0 && avail > 40) {
        float bw = avail / n - 3;
        if (bw > 160) bw = 160;
        if (bw < 40) bw = 40;
        for (TaskEntry& e : entries) {
            ImVec2 bmin(tx, btn_y), bmax(tx + bw, btn_y + btn_h);
            ImGui::SetCursorScreenPos(bmin);
            ImGui::PushID(e.id);
            bool clicked = ImGui::InvisibleButton("##task", ImVec2(bw, btn_h));
            ImGui::PopID();
            bool held = ImGui::IsItemActive() && ImGui::IsItemHovered();
            bool active = (app.active_win_id == e.id) && !*e.minimized;
            if (active || held) {
                BevelPressed(dl, bmin, bmax);
                if (active && !held)
                    dl->AddRectFilled(ImVec2(bmin.x + 2, bmin.y + 2),
                                      ImVec2(bmax.x - 2, bmax.y - 2), LIGHT);
            } else {
                BevelRaised(dl, bmin, bmax);
            }
            float o = (active || held) ? 1.0f : 0.0f;
            e.icon(dl, ImVec2(bmin.x + 3 + o, bmin.y + 4 + o));
            ImVec4 clip(bmin.x + 20, bmin.y, bmax.x - 4, bmax.y);
            float tty = bmin.y + (btn_h - ImGui::GetFontSize()) * 0.5f;
            AddTextBold(dl, ImVec2(bmin.x + 20 + o, tty + o), TEXT, e.title, &clip);
            if (clicked) {
                if (*e.minimized) {
                    *e.minimized = false;
                    *e.request_focus = kRefocusFrames;
                    app.active_win_id = e.id;
                } else if (app.active_win_id == e.id && *e.focused) {
                    *e.minimized = true;
                } else {
                    *e.request_focus = kRefocusFrames;
                    app.active_win_id = e.id;
                }
            }
            tx += bw + 3;
        }
    }

    // ---- empty taskbar area drags the HOST window (borderless mode's
    //      "title bar" — the Win95 taskbar doubles as macOS chrome) ----
    {
        float zone_w = ck_min.x - 8 - tx;
        if (zone_w > 20) {
            ImGui::SetCursorScreenPos(ImVec2(tx, mn.y + 1));
            ImGui::InvisibleButton("##hostdrag", ImVec2(zone_w, TASKBAR_H - 1));
            GLFWwindow* host = (GLFWwindow*)ImGui::GetMainViewport()->PlatformHandle;
            static double ax = 0, ay = 0;
            if (host && !glfwGetWindowMonitor(host)) {   // not in fullscreen
                if (ImGui::IsItemActivated()) glfwGetCursorPos(host, &ax, &ay);
                if (ImGui::IsItemActive()) {
                    double cx2, cy2;
                    glfwGetCursorPos(host, &cx2, &cy2);
                    if (cx2 != ax || cy2 != ay) {
                        int wx, wy;
                        glfwGetWindowPos(host, &wx, &wy);
                        glfwSetWindowPos(host, wx + (int)(cx2 - ax), wy + (int)(cy2 - ay));
                    }
                }
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    DrawStartMenu(app, top);
}
