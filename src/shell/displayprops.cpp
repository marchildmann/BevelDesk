#include "shell/displayprops.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include "imgui_internal.h"

using namespace t95;

// the classic Windows 16-color picker palette (teal included, of course)
static const ImU32 kDesktopColors[16] = {
    IM_COL32(0, 0, 0, 255),       IM_COL32(128, 0, 0, 255),
    IM_COL32(0, 128, 0, 255),     IM_COL32(128, 128, 0, 255),
    IM_COL32(0, 0, 128, 255),     IM_COL32(128, 0, 128, 255),
    IM_COL32(0, 128, 128, 255),   IM_COL32(192, 192, 192, 255),
    IM_COL32(128, 128, 128, 255), IM_COL32(255, 0, 0, 255),
    IM_COL32(0, 255, 0, 255),     IM_COL32(255, 255, 0, 255),
    IM_COL32(0, 0, 255, 255),     IM_COL32(255, 0, 255, 255),
    IM_COL32(0, 255, 255, 255),   IM_COL32(255, 255, 255, 255),
};

void OpenDisplayProperties(AppState& app) {
    app.display_props_open = true;
    app.display_props_opened_now = true;
    app.display_props_pending = app.desktop_color;
}

void DrawDisplayProperties(AppState& app) {
    if (!app.display_props_open) return;
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 dsz(300, 232);
    ImVec2 dpos(vp->Pos.x + (vp->Size.x - dsz.x) * 0.5f,
                vp->Pos.y + (vp->Size.y - dsz.y) * 0.5f - 30);
    Chrome c;
    c.title = "Display Properties";
    c.id = "##displayprops";
    c.p_open = &app.display_props_open;
    c.p_minimized = nullptr;
    c.p_maximized = nullptr;
    c.request_focus = app.display_props_opened_now;
    c.dialog = true;
    c.def_pos = dpos;
    c.def_size = dsz;
    c.min_size = dsz;
    c.max_size = dsz;
    app.display_props_opened_now = false;

    if (BeginWindow95(c)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cm = c.content_min;
        float cw = c.content_max.x - cm.x;

        // monitor preview, Win95 Display Properties style
        float mw = 110, mh = 78;
        ImVec2 mmin(cm.x + (cw - mw) * 0.5f, cm.y + 10);
        BevelRaised(dl, mmin, ImVec2(mmin.x + mw, mmin.y + mh));
        SunkenField(dl, ImVec2(mmin.x + 8, mmin.y + 6),
                    ImVec2(mmin.x + mw - 8, mmin.y + mh - 12),
                    app.display_props_pending);
        // stand
        dl->AddRectFilled(ImVec2(mmin.x + mw * 0.5f - 12, mmin.y + mh),
                          ImVec2(mmin.x + mw * 0.5f + 12, mmin.y + mh + 5), FACE);
        BevelRaised(dl, ImVec2(mmin.x + mw * 0.5f - 22, mmin.y + mh + 5),
                    ImVec2(mmin.x + mw * 0.5f + 22, mmin.y + mh + 10));

        dl->AddText(ImVec2(cm.x + 12, mmin.y + mh + 18), TEXT, "Background color:");

        // 8x2 swatch grid
        const float sw = 24, gap = 6;
        float grid_w = 8 * sw + 7 * gap;
        float gx = cm.x + (cw - grid_w) * 0.5f;
        float gy = mmin.y + mh + 34;
        for (int i = 0; i < 16; ++i) {
            float x = gx + (i % 8) * (sw + gap);
            float y = gy + (i / 8) * (sw + gap);
            ImGui::SetCursorScreenPos(ImVec2(x, y));
            ImGui::PushID(i);
            bool clicked = ImGui::InvisibleButton("##swatch", ImVec2(sw, sw));
            ImGui::PopID();
            dl->AddRectFilled(ImVec2(x, y), ImVec2(x + sw, y + sw), kDesktopColors[i]);
            dl->AddRect(ImVec2(x, y), ImVec2(x + sw, y + sw), BLACK);
            if (kDesktopColors[i] == app.display_props_pending)
                dl->AddRect(ImVec2(x - 3, y - 3), ImVec2(x + sw + 3, y + sw + 3), SEL, 0, 0, 2.0f);
            if (clicked) app.display_props_pending = kDesktopColors[i];
        }

        // OK / Cancel
        float byy = c.content_max.y - 31;
        ImGui::SetCursorScreenPos(ImVec2(c.content_max.x - 2 * 75 - 18, byy));
        if (Button("OK")) {
            app.desktop_color = app.display_props_pending;
            app.display_props_open = false;
        }
        ImGui::SetCursorScreenPos(ImVec2(c.content_max.x - 75 - 10, byy));
        if (Button("Cancel")) app.display_props_open = false;
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) app.display_props_open = false;
    }
    EndWindow95();
}
