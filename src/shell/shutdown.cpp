#include "shell/shutdown.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include "theme95/icons95.h"
#include "imgui_internal.h"

using namespace t95;

// "Shut Down Windows" — modal dialog over the iconic dithered screen fade.
void DrawShutdownDialog(AppState& app) {
    if (!app.shutdown_open) return;
    ImGuiViewport* vp = ImGui::GetMainViewport();

    // 50% black checkerboard over the whole screen (taskbar included)
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("##shutfade", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    ImDrawList* fdl = ImGui::GetWindowDrawList();
    if (app.checker_tex)
        fdl->AddImage(app.checker_tex, vp->Pos,
                      ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y),
                      ImVec2(0, 0), ImVec2(vp->Size.x / 2.0f, vp->Size.y / 2.0f));
    else
        fdl->AddRectFilled(vp->Pos, ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y),
                           IM_COL32(0, 0, 0, 128));
    // swallow clicks behind the dialog
    ImGui::SetCursorScreenPos(vp->Pos);
    ImGui::InvisibleButton("##blocker", vp->Size);
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    // the dialog itself
    ImVec2 dsz(320, 150);
    ImVec2 dpos(vp->Pos.x + (vp->Size.x - dsz.x) * 0.5f,
                vp->Pos.y + (vp->Size.y - dsz.y) * 0.5f - 20);
    Chrome c;
    c.title = "Shut Down Windows";
    c.id = "##shutdowndlg";
    c.p_open = &app.shutdown_open;
    c.p_minimized = nullptr;
    c.p_maximized = nullptr;
    c.request_focus = app.shutdown_opened_this_frame;
    c.dialog = true;
    c.def_pos = dpos;
    c.def_size = dsz;
    c.min_size = dsz;
    app.shutdown_opened_this_frame = false;
    if (BeginWindow95(c)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cm = c.content_min;
        icons95::MyComputer32(dl, ImVec2(cm.x + 14, cm.y + 14));
        dl->AddText(ImVec2(cm.x + 60, cm.y + 10), TEXT,
                    "What do you want the computer to do?");
        ImGui::SetCursorScreenPos(ImVec2(cm.x + 60, cm.y + 32));
        if (Radio("Shut down the computer", app.shutdown_choice == 0))
            app.shutdown_choice = 0;
        ImGui::SetCursorScreenPos(ImVec2(cm.x + 60, cm.y + 52));
        if (Radio("Restart the computer", app.shutdown_choice == 1))
            app.shutdown_choice = 1;

        float byy = c.content_max.y - 31;
        ImGui::SetCursorScreenPos(ImVec2(c.content_max.x - 2 * 75 - 18, byy));
        if (Button("Yes")) {
            if (app.shutdown_choice == 0) {
                app.quit_requested = true;
            } else {
                app.wins.clear();               // "restart": back to a fresh desktop
                app.active_win_id = 0;
                app.desktop_selected = -1;
            }
            app.shutdown_open = false;
        }
        ImGui::SetCursorScreenPos(ImVec2(c.content_max.x - 75 - 10, byy));
        if (Button("No")) app.shutdown_open = false;

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) app.shutdown_open = false;
    }
    EndWindow95();
}
