#include "shell/drawer.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include "imgui_internal.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

using namespace t95;

static DrawerTab* NewTab(AppState& app) {
    auto t = std::make_unique<DrawerTab>();
    t->id = app.drawer_next_id++;
    t->term.Init(80, 24);
    if (!t->pty.Spawn(80, 24)) return nullptr;   // Windows / WASM: no pty
    t->spawned = true;
    app.drawer_tabs.push_back(std::move(t));
    app.drawer_active = (int)app.drawer_tabs.size() - 1;
    return app.drawer_tabs.back().get();
}

void ToggleDrawer(AppState& app) {
    if (!app.drawer_open) {
        if (app.drawer_tabs.empty()) NewTab(app);
        if (!app.drawer_tabs.empty()) {
            app.drawer_open = true;
            app.drawer_focus_request = true;
        }
    } else {
        app.drawer_open = false;   // hide, keep the shells running
    }
}

static ImU32 VgaCol(uint8_t i) {
    return IM_COL32(kVgaPalette[i][0], kVgaPalette[i][1], kVgaPalette[i][2], 255);
}

static void EncodeUtf8(unsigned int cp, std::string& out) {
    if (cp < 0x80) out += (char)cp;
    else if (cp < 0x800) { out += (char)(0xC0 | (cp >> 6)); out += (char)(0x80 | (cp & 0x3F)); }
    else { out += (char)(0xE0 | (cp >> 12)); out += (char)(0x80 | ((cp >> 6) & 0x3F));
           out += (char)(0x80 | (cp & 0x3F)); }
}

static void CollectInput(TermGrid& t, std::string& out) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.KeyCtrl && !io.KeySuper)
        for (int i = 0; i < io.InputQueueCharacters.Size; ++i) {
            unsigned int cp = (unsigned int)io.InputQueueCharacters[i];
            if (cp >= 0x20) EncodeUtf8(cp, out);
        }
    struct K { ImGuiKey key; const char* seq; const char* app_seq; };
    static const K keys[] = {
        { ImGuiKey_Enter, "\r", nullptr }, { ImGuiKey_KeypadEnter, "\r", nullptr },
        { ImGuiKey_Backspace, "\x7f", nullptr }, { ImGuiKey_Tab, "\t", nullptr },
        { ImGuiKey_Escape, "\x1b", nullptr },
        { ImGuiKey_UpArrow, "\x1b[A", "\x1bOA" }, { ImGuiKey_DownArrow, "\x1b[B", "\x1bOB" },
        { ImGuiKey_RightArrow, "\x1b[C", "\x1bOC" }, { ImGuiKey_LeftArrow, "\x1b[D", "\x1bOD" },
        { ImGuiKey_Home, "\x1b[H", "\x1bOH" }, { ImGuiKey_End, "\x1b[F", "\x1bOF" },
        { ImGuiKey_Delete, "\x1b[3~", nullptr },
        { ImGuiKey_PageUp, "\x1b[5~", nullptr }, { ImGuiKey_PageDown, "\x1b[6~", nullptr },
    };
    for (const K& k : keys)
        if (ImGui::IsKeyPressed(k.key, true))
            out += (t.app_cursor_keys && k.app_seq) ? k.app_seq : k.seq;
    if (io.KeyCtrl)
        for (int i = 0; i < 26; ++i)
            if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_A + i), true))
                out += (char)(1 + i);
}

// Draw the active tab into the sunken field [fmin, fmax]: size the grid to
// fit, render scrollback + live screen, a Win95 scrollbar, and handle scroll
// input (mouse wheel, Shift+PageUp/Down). got_output = shell printed this frame.
static void RenderTerm(AppState& app, DrawerTab& tab, ImDrawList* dl,
                       ImVec2 fmin, ImVec2 fmax, bool focused, bool got_output) {
    ImFont* mono = FontMono ? FontMono : ImGui::GetFont();
    ImGui::PushFont(mono);
    float cw = ImGui::CalcTextSize("M").x;
    float chh = ImGui::GetFontSize() + 2;
    ImGui::PopFont();
    TermGrid& t = tab.term;

    const float pad = 4;
    const float sb_w = 16.0f;   // always reserve scrollbar width so appearing
                                // scrollback doesn't change cols (and re-init)
    ImVec2 gmin(fmin.x + pad, fmin.y + pad);
    ImVec2 gmax(fmax.x - pad - sb_w, fmax.y - pad);

    int cols = (int)((gmax.x - gmin.x) / cw);
    int rows = (int)((gmax.y - gmin.y) / chh);
    if (cols < 20) cols = 20; if (rows < 3) rows = 3;
    if (cols != tab.cols || rows != tab.rows) {
        tab.cols = cols; tab.rows = rows;
        t.Init(cols, rows); tab.pty.Resize(cols, rows);
    }

    // ---- scroll bookkeeping (scroll_px in [0, scrollback*chh]; bottom = live) ----
    float max_scroll = (float)t.scrollback.size() * chh;
    if (got_output && tab.stick_bottom) tab.scroll_px = max_scroll;
    ImGuiIO& io = ImGui::GetIO();
    bool hover = io.MousePos.x >= fmin.x && io.MousePos.x <= fmax.x &&
                 io.MousePos.y >= fmin.y && io.MousePos.y <= fmax.y;
    if (hover && !io.KeyCtrl && io.MouseWheel != 0.0f) {
        tab.scroll_px -= io.MouseWheel * chh * 3.0f;   // wheel up = older (scroll back)
        io.MouseWheel = 0.0f;
    }
    if (focused && io.KeyShift) {                       // Shift+PageUp/Down = a page
        float page = rows * chh;
        if (ImGui::IsKeyPressed(ImGuiKey_PageUp, true)) tab.scroll_px -= page;
        if (ImGui::IsKeyPressed(ImGuiKey_PageDown, true)) tab.scroll_px += page;
    }
    if (tab.scroll_px < 0) tab.scroll_px = 0;
    if (tab.scroll_px > max_scroll) tab.scroll_px = max_scroll;
    tab.stick_bottom = tab.scroll_px >= max_scroll - 0.5f;

    // typing snaps back to the live screen
    if (focused) {
        std::string out; CollectInput(t, out);
        if (!out.empty()) {
            tab.pty.Write(out.data(), out.size());
            tab.scroll_px = max_scroll; tab.stick_bottom = true;
        }
    }

    // ---- render scrollback + live screen ----
    ImGui::PushFont(mono);
    int first_line = (int)(tab.scroll_px / chh + 0.5f);
    int total_lines = (int)t.scrollback.size() + t.rows;
    for (int vr = 0; vr < t.rows; ++vr) {
        int line = first_line + vr;
        if (line >= total_lines) break;
        const TermCell* row = (line < (int)t.scrollback.size())
                                  ? t.scrollback[(size_t)line].data()
                                  : t.Row(line - (int)t.scrollback.size());
        float ry = gmin.y + vr * chh;
        int x = 0;
        while (x < t.cols) {
            uint8_t fg = row[x].fg, bg = row[x].bg;
            int end = x; std::string text;
            while (end < t.cols && row[end].fg == fg && row[end].bg == bg) {
                EncodeUtf8(row[end].ch, text); ++end;
            }
            float rx = gmin.x + x * cw;
            if (bg != 0)
                dl->AddRectFilled(ImVec2(rx, ry), ImVec2(gmin.x + end * cw, ry + chh), VgaCol(bg));
            if (text.find_first_not_of(' ') != std::string::npos)
                dl->AddText(ImVec2(rx, ry + 1), VgaCol(fg), text.c_str());
            x = end;
        }
    }
    bool at_bottom = tab.scroll_px >= max_scroll - 0.5f;
    if (t.cursor_visible && at_bottom && focused && std::fmod(ImGui::GetTime(), 1.0) < 0.6) {
        float cxp = gmin.x + t.cx * cw, cyp = gmin.y + (t.scrollback.size() + t.cy - first_line) * chh;
        dl->AddRectFilled(ImVec2(cxp, cyp), ImVec2(cxp + cw, cyp + chh), IM_COL32(170, 170, 170, 255));
    }
    ImGui::PopFont();

    // ---- Win95 scrollbar (always present; full thumb when nothing to scroll) ----
    {
        float ns = ScrollBarV("##drawersb", ImVec2(fmax.x - 2 - 16, fmin.y + 2),
                              ImVec2(fmax.x - 2, fmax.y - 2),
                              t.rows * chh, total_lines * chh, tab.scroll_px);
        if (ns != tab.scroll_px) { tab.scroll_px = ns; tab.stick_bottom = ns >= max_scroll - 0.5f; }
    }
}

void DrawTerminalDrawer(AppState& app, float drawer_top) {
    // pump EVERY tab's shell each frame (even hidden), and reap exited ones
    bool active_got_output = false;
    for (size_t i = 0; i < app.drawer_tabs.size();) {
        DrawerTab& tab = *app.drawer_tabs[i];
        bool exited = false, got = false;
        if (tab.spawned) {
            char buf[8192]; int n;
            while ((n = tab.pty.Read(buf, sizeof(buf))) > 0) { tab.term.Feed(buf, (size_t)n); got = true; }
            if (n == 0) exited = true;
            if (!tab.term.reply.empty()) {
                tab.pty.Write(tab.term.reply.data(), tab.term.reply.size());
                tab.term.reply.clear();
            }
        }
        if ((int)i == app.drawer_active && got) active_got_output = true;
        if (exited) {
            app.drawer_tabs.erase(app.drawer_tabs.begin() + (long)i);
            if (app.drawer_active >= (int)app.drawer_tabs.size())
                app.drawer_active = (int)app.drawer_tabs.size() - 1;
        } else ++i;
    }
    app.drawer_focused = false;
    if (app.drawer_tabs.empty()) { app.drawer_open = false; return; }
    if (!app.drawer_open) return;
    if (app.drawer_active < 0) app.drawer_active = 0;

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, drawer_top));
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, app.drawer_height));
    if (app.drawer_focus_request) { ImGui::SetNextWindowFocus(); app.drawer_focus_request = false; }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(FACE));
    ImGui::Begin("##drawer", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings);
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    bool win_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    app.drawer_focused = win_focused;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mn = ImGui::GetWindowPos();
    ImVec2 mx(mn.x + vp->Size.x, mn.y + app.drawer_height);
    dl->AddRectFilled(mn, mx, FACE);

    // ---- tab strip ----
    const float th = 21;
    float tabx = mn.x + 3, taby = mn.y + 3;
    int close_tab = -1, new_tab = 0;
    for (int i = 0; i < (int)app.drawer_tabs.size(); ++i) {
        bool active = (i == app.drawer_active);
        float tw = 104;
        ImVec2 t0(tabx, taby), t1(tabx + tw, taby + th);
        ImVec2 xb0(t1.x - 16, t0.y + 4), xb1(t1.x - 5, t0.y + 15);
        int id = app.drawer_tabs[i]->id;

        // Submit the small close box FIRST so it wins the click over the tab
        // body it overlaps (gotcha: earlier item claims the press).
        ImGui::SetCursorScreenPos(xb0);
        ImGui::PushID(2000 + id);
        bool xclick = ImGui::InvisibleButton("##x", ImVec2(11, 11));
        ImGui::PopID();
        // tab body second
        ImGui::SetCursorScreenPos(t0);
        ImGui::PushID(id);
        bool clicked = ImGui::InvisibleButton("##tab", ImVec2(tw, th));
        ImGui::PopID();

        if (active) BevelPressed(dl, t0, t1); else BevelRaised(dl, t0, t1);
        float o = active ? 1.0f : 0.0f;
        char label[24]; std::snprintf(label, sizeof(label), "Terminal %d", id);
        ImVec4 clip(t0.x + 6, t0.y, t1.x - 18, t1.y);
        dl->AddText(nullptr, 0.0f, ImVec2(t0.x + 7 + o, t0.y + 4 + o), TEXT, label, nullptr, 0.0f, &clip);
        dl->AddLine(ImVec2(xb0.x + 2, xb0.y + 2), ImVec2(xb1.x - 2, xb1.y - 2), TEXT);
        dl->AddLine(ImVec2(xb1.x - 2, xb0.y + 2), ImVec2(xb0.x + 2, xb1.y - 2), TEXT);
        if (xclick) close_tab = i;
        else if (clicked) { app.drawer_active = i; app.drawer_focus_request = true; }
        tabx += tw + 2;
    }
    // "+" new-tab button
    {
        ImVec2 p0(tabx, taby), p1(tabx + th, taby + th);
        ImGui::SetCursorScreenPos(p0);
        bool add = ImGui::InvisibleButton("##newtab", ImVec2(th, th));
        bool held = ImGui::IsItemActive() && ImGui::IsItemHovered();
        if (held) BevelPressed(dl, p0, p1); else BevelRaised(dl, p0, p1);
        float cx = (p0.x + p1.x) * 0.5f, cy = (p0.y + p1.y) * 0.5f;
        dl->AddLine(ImVec2(cx - 4, cy), ImVec2(cx + 4, cy), TEXT);
        dl->AddLine(ImVec2(cx, cy - 4), ImVec2(cx, cy + 4), TEXT);
        if (add) new_tab = 1;
    }

    // Cmd+T new tab / Cmd+W close tab when focused (won't reach the shell)
    if (win_focused) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_T, false)) new_tab = 1;
        if (io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_W, false)) close_tab = app.drawer_active;
    }

    // ---- terminal field for the active tab ----
    ImVec2 fmin(mn.x + 3, taby + th + 3), fmax(mx.x - 3, mx.y - 3);
    SunkenField(dl, fmin, fmax, IM_COL32(0, 0, 0, 255));
    RenderTerm(app, *app.drawer_tabs[app.drawer_active], dl, fmin, fmax,
               win_focused, active_got_output);

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    // apply tab add/close after drawing (avoid mutating mid-iteration)
    if (new_tab) NewTab(app);
    if (close_tab >= 0 && close_tab < (int)app.drawer_tabs.size()) {
        app.drawer_tabs.erase(app.drawer_tabs.begin() + close_tab);
        if (app.drawer_active >= (int)app.drawer_tabs.size())
            app.drawer_active = (int)app.drawer_tabs.size() - 1;
        if (app.drawer_tabs.empty()) app.drawer_open = false;
        else app.drawer_focus_request = true;
    }
}
