#include "shell/run.h"
#include "shell/dosprompt.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include "theme95/icons95.h"
#include "imgui_internal.h"
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;
using namespace t95;

void OpenRun(AppState& app) {
    app.run_open = true;
    app.run_opened_now = true;
}

// Act on the Run text: a directory opens in Explorer; anything else is run as
// a command in a fresh MS-DOS Prompt.
static void Execute(AppState& app) {
    std::string cmd = app.run_buf;
    // trim
    size_t a = cmd.find_first_not_of(" \t");
    size_t b = cmd.find_last_not_of(" \t");
    if (a == std::string::npos) { app.run_open = false; return; }
    cmd = cmd.substr(a, b - a + 1);

    std::error_code ec;
    fs::path p(cmd);
    if (fs::is_directory(p, ec)) {
        app.OpenExplorer(p);
    } else {
        OpenDosPrompt(app);
        if (!app.dos_wins.empty()) {
            std::string line = cmd + "\n";
            app.dos_wins.back()->pty.Write(line.data(), line.size());
        }
    }
    app.run_buf[0] = 0;
    app.run_open = false;
}

void DrawRun(AppState& app) {
    if (!app.run_open) return;
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 dsz(360, 148);
    ImVec2 dpos(vp->Pos.x + (vp->Size.x - dsz.x) * 0.5f,
                vp->Pos.y + (vp->Size.y - dsz.y) * 0.5f - 40);
    Chrome c;
    c.title = "Run";
    c.id = "##run";
    c.p_open = &app.run_open;
    c.p_minimized = nullptr;
    c.p_maximized = nullptr;
    c.request_focus = app.run_opened_now;
    c.dialog = true;
    c.def_pos = dpos;
    c.def_size = dsz;
    c.min_size = dsz;
    c.max_size = dsz;
    bool just_opened = app.run_opened_now;
    app.run_opened_now = false;

    if (BeginWindow95(c)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cm = c.content_min;

        icons95::MiniDos14(dl, ImVec2(cm.x + 12, cm.y + 14));
        dl->AddText(ImVec2(cm.x + 34, cm.y + 8), TEXT,
                    "Type the name of a program, folder, or");
        dl->AddText(ImVec2(cm.x + 34, cm.y + 24), TEXT,
                    "document, and BevelDesk will open it.");

        // input field, styled sunken; ImGui::InputText draws the text/caret
        ImVec2 fmin(cm.x + 34, cm.y + 46), fmax(c.content_max.x - 12, cm.y + 68);
        SunkenField(dl, fmin, fmax, WINBG);
        ImGui::SetCursorScreenPos(ImVec2(fmin.x + 4, fmin.y + 4));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::SetNextItemWidth(fmax.x - fmin.x - 8);
        if (just_opened) ImGui::SetKeyboardFocusHere();
        bool entered = ImGui::InputText("##runinput", app.run_buf, sizeof(app.run_buf),
                                        ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        // OK / Cancel / Browse
        float byy = c.content_max.y - 31;
        float bw = 78, gap = 6;
        float bx = c.content_max.x - bw;                 // Browse (rightmost)
        ImGui::SetCursorScreenPos(ImVec2(bx - 2 * (bw + gap), byy));
        bool ok = Button("OK", ImVec2(bw, 0));
        ImGui::SetCursorScreenPos(ImVec2(bx - (bw + gap), byy));
        if (Button("Cancel", ImVec2(bw, 0))) { app.run_buf[0] = 0; app.run_open = false; }
        ImGui::SetCursorScreenPos(ImVec2(bx, byy));
        if (Button("Browse...", ImVec2(bw, 0))) {
            const char* home = std::getenv("HOME");
            app.OpenExplorer(home ? home : "/");   // stand-in for a file picker
            app.run_open = false;
        }
        if (ok || entered) Execute(app);
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { app.run_buf[0] = 0; app.run_open = false; }
    }
    EndWindow95();
}
