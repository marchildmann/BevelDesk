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
    app.display_props_original = app.desktop_color;   // revert target for Cancel
}

void DrawDisplayProperties(AppState& app) {
    if (!app.display_props_open) return;
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImVec2 dsz(310, 340);
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

    bool ok_clicked = false;
    if (BeginWindow95(c)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cm = c.content_min;
        float cw = c.content_max.x - cm.x;

        // ---- "Background" tab over a page panel ----
        float pad = 8;
        ImVec2 page_mn(cm.x + pad, cm.y + 26);
        ImVec2 page_mx(c.content_max.x - pad, c.content_max.y - 44);
        WindowFrame(dl, page_mn, page_mx);

        // active tab: raised, top + left highlight, right shadow/black, NO bottom
        // edge — it fills over the page's top border so the two read as joined.
        ImVec2 tab_mn(page_mn.x + 6, cm.y + 6);
        ImVec2 tab_mx(tab_mn.x + 90, page_mn.y + 2);   // extends 2px into the page
        dl->AddRectFilled(tab_mn, tab_mx, FACE);       // covers page top border here
        dl->AddRectFilled(tab_mn, ImVec2(tab_mx.x - 2, tab_mn.y + 1), HILIGHT);   // top
        dl->AddRectFilled(tab_mn, ImVec2(tab_mn.x + 1, tab_mx.y), HILIGHT);       // left
        dl->AddRectFilled(ImVec2(tab_mx.x - 2, tab_mn.y), ImVec2(tab_mx.x - 1, tab_mx.y), SHADOW);
        dl->AddRectFilled(ImVec2(tab_mx.x - 1, tab_mn.y), ImVec2(tab_mx.x, tab_mx.y), DKSHADOW);
        dl->AddText(ImVec2(tab_mn.x + 11, tab_mn.y + 4), TEXT, "Background");

        // ---- monitor preview, centered near the page top ----
        float mw = 118, mh = 84;
        ImVec2 mmin(cm.x + (cw - mw) * 0.5f, page_mn.y + 16);
        BevelRaised(dl, mmin, ImVec2(mmin.x + mw, mmin.y + mh));
        SunkenField(dl, ImVec2(mmin.x + 9, mmin.y + 8),
                    ImVec2(mmin.x + mw - 9, mmin.y + mh - 16),
                    app.display_props_pending);
        // stand
        dl->AddRectFilled(ImVec2(mmin.x + mw * 0.5f - 13, mmin.y + mh),
                          ImVec2(mmin.x + mw * 0.5f + 13, mmin.y + mh + 6), FACE);
        BevelRaised(dl, ImVec2(mmin.x + mw * 0.5f - 24, mmin.y + mh + 6),
                    ImVec2(mmin.x + mw * 0.5f + 24, mmin.y + mh + 11));

        // ---- label + 8x2 swatch grid ----
        float ly = mmin.y + mh + 22;
        dl->AddText(ImVec2(page_mn.x + 10, ly), TEXT, "Background color:");
        const float sw = 24, gap = 7;
        float grid_w = 8 * sw + 7 * gap;
        float gx = cm.x + (cw - grid_w) * 0.5f;
        float gy = ly + 20;
        for (int i = 0; i < 16; ++i) {
            float x = gx + (i % 8) * (sw + gap);
            float y = gy + (i / 8) * (sw + gap);
            ImGui::SetCursorScreenPos(ImVec2(x, y));
            ImGui::PushID(i);
            bool clicked = ImGui::InvisibleButton("##swatch", ImVec2(sw, sw));
            ImGui::PopID();
            SunkenField(dl, ImVec2(x, y), ImVec2(x + sw, y + sw), kDesktopColors[i]);
            if (kDesktopColors[i] == app.display_props_pending)
                dl->AddRect(ImVec2(x - 3, y - 3), ImVec2(x + sw + 3, y + sw + 3),
                            BLACK, 0, 0, 2.0f);
            if (clicked) {
                app.display_props_pending = kDesktopColors[i];
                app.desktop_color = kDesktopColors[i];   // live apply to the desktop
            }
        }

        // ---- OK / Cancel, bottom-right, below the page ----
        float byy = c.content_max.y - 34;
        ImGui::SetCursorScreenPos(ImVec2(c.content_max.x - 2 * 75 - 16, byy));
        if (Button("OK")) {
            app.desktop_color = app.display_props_pending;  // commit (already live)
            app.display_props_open = false;
            ok_clicked = true;
        }
        ImGui::SetCursorScreenPos(ImVec2(c.content_max.x - 75 - 8, byy));
        if (Button("Cancel")) app.display_props_open = false;
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) app.display_props_open = false;
    }
    EndWindow95();

    // closed by anything other than OK (Cancel / Esc / the X) reverts the
    // live-previewed color back to what it was when the dialog opened
    if (!app.display_props_open && !ok_clicked)
        app.desktop_color = app.display_props_original;
}
