#include "shell/dosprompt.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include "theme95/icons95.h"
#include <cmath>
#include <cstring>
#include <string>

using namespace t95;

static ImU32 VgaCol(uint8_t i) {
    return IM_COL32(kVgaPalette[i][0], kVgaPalette[i][1], kVgaPalette[i][2], 255);
}

void OpenDosPrompt(AppState& app) {
    auto w = std::make_unique<DosWin>();
    w->id = app.next_id++;
    w->imgui_id = "##dos" + std::to_string(w->id);
    w->term.Init(80, 25);
    if (!w->pty.Spawn(w->term.cols, w->term.rows)) return; // Windows / failure
    int n = (int)app.dos_wins.size();
    w->def_pos = ImVec2(160.0f + 26.0f * n, 120.0f + 26.0f * n);
    w->request_focus = kRefocusFrames;
    app.active_win_id = w->id;
    app.dos_wins.push_back(std::move(w));
}

static void EncodeUtf8(unsigned int cp, std::string& out) {
    if (cp < 0x80) out += (char)cp;
    else if (cp < 0x800) {
        out += (char)(0xC0 | (cp >> 6));
        out += (char)(0x80 | (cp & 0x3F));
    } else {
        out += (char)(0xE0 | (cp >> 12));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    }
}

// Translate this frame's keyboard input into terminal bytes.
static void CollectInput(DosWin& w, std::string& out) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.KeyCtrl && !io.KeySuper)
        for (int i = 0; i < io.InputQueueCharacters.Size; ++i) {
            unsigned int cp = (unsigned int)io.InputQueueCharacters[i];
            if (cp >= 0x20) EncodeUtf8(cp, out);
        }

    struct KeySeq { ImGuiKey key; const char* seq; const char* app_seq; };
    static const KeySeq keys[] = {
        { ImGuiKey_Enter, "\r", nullptr },
        { ImGuiKey_KeypadEnter, "\r", nullptr },
        { ImGuiKey_Backspace, "\x7f", nullptr },
        { ImGuiKey_Tab, "\t", nullptr },
        { ImGuiKey_Escape, "\x1b", nullptr },
        { ImGuiKey_UpArrow, "\x1b[A", "\x1bOA" },
        { ImGuiKey_DownArrow, "\x1b[B", "\x1bOB" },
        { ImGuiKey_RightArrow, "\x1b[C", "\x1bOC" },
        { ImGuiKey_LeftArrow, "\x1b[D", "\x1bOD" },
        { ImGuiKey_Home, "\x1b[H", "\x1bOH" },
        { ImGuiKey_End, "\x1b[F", "\x1bOF" },
        { ImGuiKey_Delete, "\x1b[3~", nullptr },
        { ImGuiKey_PageUp, "\x1b[5~", nullptr },
        { ImGuiKey_PageDown, "\x1b[6~", nullptr },
    };
    for (const KeySeq& k : keys)
        if (ImGui::IsKeyPressed(k.key, true))
            out += (w.term.app_cursor_keys && k.app_seq) ? k.app_seq : k.seq;

    if (io.KeyCtrl)
        for (int i = 0; i < 26; ++i)
            if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_A + i), true))
                out += (char)(1 + i);        // ^A..^Z (^C interrupts, ^D EOF, ...)
}

static void DrawDosPrompt(AppState& app, DosWin& w) {
    TermGrid& t = w.term;

    // ---- pump the pty (even while minimized, so the shell never stalls) ----
    char buf[8192];
    int n;
    bool got_output = false;
    while ((n = w.pty.Read(buf, sizeof(buf))) > 0) {
        t.Feed(buf, (size_t)n);
        got_output = true;
    }
    if (n == 0) { w.open = false; return; }  // shell exited -> close window
    if (!t.reply.empty()) { w.pty.Write(t.reply.data(), t.reply.size()); t.reply.clear(); }
    if (!t.title.empty()) w.title = t.title;
    if (w.minimized) return;

    // ---- fixed window size from the mono cell metrics ----
    ImFont* mono = FontMono ? FontMono : ImGui::GetFont();
    ImGui::PushFont(mono);
    float cw = ImGui::CalcTextSize("M").x;
    float chh = ImGui::GetFontSize() + 2;
    ImGui::PopFont();
    const float pad = 2;
    float grid_w = t.cols * cw, grid_h = t.rows * chh;
    ImVec2 win_size(WIN_BORDER * 2 + 4 + pad * 2 + grid_w + 16,
                    WIN_BORDER * 2 + TITLEBAR_H + 1 + 4 + pad * 2 + grid_h);

    Chrome c;
    c.title = w.title.c_str();
    c.id = w.imgui_id.c_str();
    c.p_open = &w.open;
    c.p_minimized = &w.minimized;
    c.p_maximized = nullptr;              // fixed 80x25, like the real thing
    c.console = true;                     // system menu gains "Properties"
    c.request_focus = w.request_focus > 0 && !app.ModalOpen();
    c.def_pos = w.def_pos;
    c.def_size = win_size;
    c.min_size = win_size;
    c.max_size = win_size;
    if (w.request_focus > 0) --w.request_focus;

    if (!BeginWindow95(c)) { EndWindow95(); return; }
    w.focused = c.focused;
    if (w.focused) app.active_win_id = w.id;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 fmin = c.content_min, fmax = c.content_max;
    SunkenField(dl, fmin, fmax, IM_COL32(0, 0, 0, 255));

    // ---- keyboard -> pty ----
    if (w.focused) {
        std::string out;
        CollectInput(w, out);
        if (!out.empty()) {
            w.pty.Write(out.data(), out.size());
            w.stick_bottom = true;        // typing snaps back to the live screen
        }
    }

    // ---- scrollback state ----
    float max_scroll = (float)t.scrollback.size() * chh;
    if (got_output && w.stick_bottom) w.scroll_px = max_scroll;
    if (w.scroll_px > max_scroll) w.scroll_px = max_scroll;

    ImVec2 gmin(fmin.x + 2 + pad, fmin.y + 2 + pad);
    int first_line = (int)(w.scroll_px / chh + 0.5f);
    int total_lines = (int)t.scrollback.size() + t.rows;

    ImGui::PushFont(mono);
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
            int run_end = x;
            std::string text;
            while (run_end < t.cols && row[run_end].fg == fg && row[run_end].bg == bg) {
                EncodeUtf8(row[run_end].ch, text);
                ++run_end;
            }
            float rx = gmin.x + x * cw;
            if (bg != 0)
                dl->AddRectFilled(ImVec2(rx, ry), ImVec2(gmin.x + run_end * cw, ry + chh),
                                  VgaCol(bg));
            if (text.find_first_not_of(' ') != std::string::npos)
                dl->AddText(ImVec2(rx, ry + 1), VgaCol(fg), text.c_str());
            x = run_end;
        }
    }

    // blinking block cursor (only on the live screen, when focused)
    bool at_bottom = w.scroll_px >= max_scroll - 0.5f;
    if (t.cursor_visible && at_bottom && w.focused &&
        std::fmod(ImGui::GetTime(), 1.0) < 0.6) {
        float cxp = gmin.x + t.cx * cw;
        float cyp = gmin.y + ((int)t.scrollback.size() + t.cy - first_line) * chh;
        dl->AddRectFilled(ImVec2(cxp, cyp), ImVec2(cxp + cw, cyp + chh),
                          IM_COL32(170, 170, 170, 255));
    }
    ImGui::PopFont();

    // ---- scrollback scrollbar ----
    if (!t.scrollback.empty()) {
        float view_h = t.rows * chh;
        float content_h = total_lines * chh;
        float ns = ScrollBarV("##dossb", ImVec2(fmax.x - 2 - 16, fmin.y + 2),
                              ImVec2(fmax.x - 2, fmax.y - 2),
                              view_h, content_h, w.scroll_px);
        if (ns != w.scroll_px) {
            w.scroll_px = ns;
            w.stick_bottom = ns >= max_scroll - 0.5f;
        }
    }

    EndWindow95();
}

void DrawDosPrompts(AppState& app) {
    for (auto& w : app.dos_wins)
        if (w->open) DrawDosPrompt(app, *w);
    for (size_t i = 0; i < app.dos_wins.size();) {
        if (!app.dos_wins[i]->open) app.dos_wins.erase(app.dos_wins.begin() + (long)i);
        else ++i;
    }
}
