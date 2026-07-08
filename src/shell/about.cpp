#include "shell/about.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include "theme95/icons95.h"
#include "imgui_internal.h"

using namespace t95;

void OpenAbout(AppState& app) {
    app.about_open = true;
    app.about_opened_now = true;
}

void DrawAbout(AppState& app) {
    if (!app.about_open) return;
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 dsz(340, 190);
    ImVec2 dpos(vp->Pos.x + (vp->Size.x - dsz.x) * 0.5f,
                vp->Pos.y + (vp->Size.y - dsz.y) * 0.5f - 20);
    Chrome c;
    c.title = "About BevelDesk";
    c.id = "##about";
    c.p_open = &app.about_open;
    c.p_minimized = nullptr;
    c.p_maximized = nullptr;
    c.request_focus = app.about_opened_now;
    c.dialog = true;
    c.def_pos = dpos;
    c.def_size = dsz;
    c.min_size = dsz;
    c.max_size = dsz;
    app.about_opened_now = false;

    if (BeginWindow95(c)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cm = c.content_min;

        // big BevelDesk mark, top-left
        float s = 2.6f;
        ImVec2 lp(cm.x + 18, cm.y + 16);
        auto tile = [&](float x, float y, ImU32 fill) {
            ImVec2 a(lp.x + x * s, lp.y + y * s), b(lp.x + (x + 6) * s, lp.y + (y + 6) * s);
            dl->AddRectFilled(a, b, fill);
            Edge(dl, a, b, HILIGHT, DKSHADOW);
        };
        tile(0, 0, DESKTOP); tile(7, 0, FACE); tile(0, 7, FACE); tile(7, 7, FACE);

        float tx = cm.x + 70;
        AddTextBold(dl, ImVec2(tx, cm.y + 14), TEXT, "BevelDesk 1.0");
        dl->AddText(ImVec2(tx, cm.y + 34), TEXT, "A Windows 95-style desktop,");
        dl->AddText(ImVec2(tx, cm.y + 50), TEXT, "hand-drawn in Dear ImGui.");
        dl->AddText(ImVec2(tx, cm.y + 74), GRAYTEXT, "Dear ImGui by Omar Cornut (@ocornut)");
        dl->AddText(ImVec2(tx, cm.y + 90), GRAYTEXT, "GLFW - OpenGL - stb - MIT License");

        // separator groove above the button
        ThinSunken(dl, ImVec2(cm.x + 8, c.content_max.y - 40),
                   ImVec2(c.content_max.x - 8, c.content_max.y - 38));

        ImGui::SetCursorScreenPos(ImVec2(c.content_max.x - 85, c.content_max.y - 31));
        if (Button("OK")) app.about_open = false;
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) app.about_open = false;
    }
    EndWindow95();
}
