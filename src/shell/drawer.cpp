#include "shell/drawer.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include "imgui_internal.h"
#include <cstring>
#include <string>

using namespace t95;

void ToggleDrawer(AppState& app) {
    if (!app.drawer_open) {
        if (!app.drawer_spawned) {
            app.drawer_term.Init(80, 24);
            if (app.drawer_pty.Spawn(80, 24)) app.drawer_spawned = true;
        }
        app.drawer_open = true;
        app.drawer_focus_request = true;
    } else {
        app.drawer_open = false;   // hide, keep the shell running
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

// keyboard -> terminal bytes (same subset the MS-DOS Prompt uses)
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
                out += (char)(1 + i);  // ^A..^Z  (^C, ^D, ^L, ...)
}

void DrawTerminalDrawer(AppState& app, float drawer_top) {
    TermGrid& t = app.drawer_term;

    // pump the shell every frame (even hidden) so it never blocks on a full pipe
    if (app.drawer_spawned) {
        char buf[8192]; int n;
        while ((n = app.drawer_pty.Read(buf, sizeof(buf))) > 0) t.Feed(buf, (size_t)n);
        if (n == 0) { app.drawer_spawned = false; app.drawer_open = false; } // shell exited
        if (!t.reply.empty()) { app.drawer_pty.Write(t.reply.data(), t.reply.size()); t.reply.clear(); }
    }
    if (!app.drawer_open) return;

    ImGuiViewport* vp = ImGui::GetMainViewport();
    float dh = app.drawer_height;
    ImVec2 dmin(vp->Pos.x, drawer_top);
    ImVec2 dsz(vp->Size.x, dh);

    ImGui::SetNextWindowPos(dmin);
    ImGui::SetNextWindowSize(dsz);
    if (app.drawer_focus_request) { ImGui::SetNextWindowFocus(); app.drawer_focus_request = false; }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(FACE));
    ImGui::Begin("##drawer", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings);
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    bool focused = ImGui::IsWindowFocused();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mn = ImGui::GetWindowPos();
    ImVec2 mx(mn.x + dsz.x, mn.y + dsz.y);

    // beveled frame + sunken black terminal field
    dl->AddRectFilled(mn, mx, FACE);
    ImVec2 fmin(mn.x + 3, mn.y + 3), fmax(mx.x - 3, mx.y - 3);
    SunkenField(dl, fmin, fmax, IM_COL32(0, 0, 0, 255));

    // size the grid to the field, tell the shell if it changed
    ImFont* mono = FontMono ? FontMono : ImGui::GetFont();
    ImGui::PushFont(mono);
    float cw = ImGui::CalcTextSize("M").x;
    float chh = ImGui::GetFontSize() + 2;
    ImGui::PopFont();
    float pad = 4;
    int cols = (int)((fmax.x - fmin.x - pad * 2) / cw);
    int rows = (int)((fmax.y - fmin.y - pad * 2) / chh);
    if (cols < 20) cols = 20; if (rows < 4) rows = 4;
    if (cols != app.drawer_cols || rows != app.drawer_rows) {
        app.drawer_cols = cols; app.drawer_rows = rows;
        t.Init(cols, rows);
        app.drawer_pty.Resize(cols, rows);
    }

    // keyboard -> shell (only when the drawer has focus)
    if (focused) {
        std::string out;
        CollectInput(t, out);
        if (!out.empty()) app.drawer_pty.Write(out.data(), out.size());
    }

    // render the live screen (bottom-anchored; scrollback not shown here)
    ImVec2 gmin(fmin.x + pad, fmin.y + pad);
    ImGui::PushFont(mono);
    for (int r = 0; r < t.rows; ++r) {
        const TermCell* row = t.Row(r);
        float ry = gmin.y + r * chh;
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
    // blinking block cursor
    if (t.cursor_visible && focused && std::fmod(ImGui::GetTime(), 1.0) < 0.6) {
        float cxp = gmin.x + t.cx * cw, cyp = gmin.y + t.cy * chh;
        dl->AddRectFilled(ImVec2(cxp, cyp), ImVec2(cxp + cw, cyp + chh),
                          IM_COL32(170, 170, 170, 255));
    }
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}
