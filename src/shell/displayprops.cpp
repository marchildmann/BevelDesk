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

// One Win95 property-sheet tab with 1px chamfered top corners. The active tab
// is raised (its top sits `pop` px higher) and its fill covers the page's top
// border so the two read as joined; inactive tabs sit lower with the page
// border showing as their bottom edge.
static void DrawTab(ImDrawList* dl, float x, float w, float top_y, float page_top,
                    const char* label, bool active) {
    float t = active ? top_y : top_y + 2;               // inactive tops are lower
    float b = active ? page_top + 1 : page_top;         // active merges into the page interior
    // fill, inset 1px at the top for the chamfered corners
    dl->AddRectFilled(ImVec2(x + 1, t), ImVec2(x + w - 1, b), FACE);
    dl->AddRectFilled(ImVec2(x, t + 1), ImVec2(x + w, b), FACE);
    // top highlight (between the cut corners) + left highlight edge
    dl->AddRectFilled(ImVec2(x + 2, t), ImVec2(x + w - 2, t + 1), HILIGHT);
    dl->AddRectFilled(ImVec2(x + 1, t + 1), ImVec2(x + 2, t + 2), HILIGHT); // TL chamfer
    dl->AddRectFilled(ImVec2(x, t + 2), ImVec2(x + 1, b), HILIGHT);
    // right side: shadow then black, with the TR chamfer stepped in 1px
    dl->AddRectFilled(ImVec2(x + w - 2, t + 1), ImVec2(x + w - 1, t + 2), SHADOW); // TR chamfer
    dl->AddRectFilled(ImVec2(x + w - 2, t + 2), ImVec2(x + w - 1, b), SHADOW);
    dl->AddRectFilled(ImVec2(x + w - 1, t + 2), ImVec2(x + w, b), DKSHADOW);
    dl->AddText(ImVec2(x + 9, t + 4), TEXT, label);
}

void OpenDisplayProperties(AppState& app) {
    app.display_props_open = true;
    app.display_props_opened_now = true;
    app.display_props_pending = app.desktop_color;
    app.display_props_original = app.desktop_color;   // revert target for Cancel
}

void DrawDisplayProperties(AppState& app) {
    if (!app.display_props_open) return;

    Chrome c = CenteredDialog("Display Properties", "##displayprops",
                              &app.display_props_open, app.display_props_opened_now,
                              ImVec2(380, 344), -30.0f);
    app.display_props_opened_now = false;

    bool ok_clicked = false;
    if (BeginWindow95(c)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cm = c.content_min;
        float cw = c.content_max.x - cm.x;

        // ---- tab strip over a page panel (Background active; the rest are
        //      authentic decorative tabs, like the real Display Properties) ----
        float pad = 8;
        ImVec2 page_mn(cm.x + pad, cm.y + 28);
        ImVec2 page_mx(c.content_max.x - pad, c.content_max.y - 44);
        // tab-control page border: 1px white top/left (so the active tab's white
        // left edge lines up exactly), 2px shadow/black bottom/right. The active
        // tab overdraws the top border along its own width.
        dl->AddRectFilled(page_mn, page_mx, FACE);
        dl->AddRectFilled(page_mn, ImVec2(page_mx.x - 1, page_mn.y + 1), HILIGHT); // top
        dl->AddRectFilled(page_mn, ImVec2(page_mn.x + 1, page_mx.y - 1), HILIGHT); // left
        dl->AddRectFilled(ImVec2(page_mx.x - 2, page_mn.y), ImVec2(page_mx.x - 1, page_mx.y), SHADOW);
        dl->AddRectFilled(ImVec2(page_mx.x - 1, page_mn.y), ImVec2(page_mx.x, page_mx.y), DKSHADOW);
        dl->AddRectFilled(ImVec2(page_mn.x, page_mx.y - 2), ImVec2(page_mx.x, page_mx.y - 1), SHADOW);
        dl->AddRectFilled(ImVec2(page_mn.x, page_mx.y - 1), ImVec2(page_mx.x, page_mx.y), DKSHADOW);

        static const char* tabs[] = { "Background", "Screen Saver", "Appearance", "Settings" };
        float tab_top = cm.y + 6;
        // draw inactive tabs first, active one last so it overlaps at the seam
        for (int pass = 0; pass < 2; ++pass) {
            float x = page_mn.x;
            for (int ti = 0; ti < 4; ++ti) {
                float tw = ImGui::CalcTextSize(tabs[ti]).x + 18;
                bool active = (ti == 0);
                if (active == (pass == 1))
                    DrawTab(dl, x, tw, tab_top, page_mn.y, tabs[ti], active);
                x += tw - 1;   // 1px overlap between adjacent tabs
            }
        }

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

        // ---- OK / Cancel / Apply, bottom-right, below the page ----
        // Apply enables once the preview differs from the last committed color;
        // it commits (moving the Cancel-revert baseline) without closing.
        bool changed = app.display_props_pending != app.display_props_original;
        float byy = c.content_max.y - 34;
        float bw = 75, bgap = 6;
        float ax = c.content_max.x - bw;                 // Apply (rightmost)
        ImGui::SetCursorScreenPos(ImVec2(ax - 2 * (bw + bgap), byy));
        if (Button("OK")) {
            app.desktop_color = app.display_props_pending;  // commit (already live)
            app.display_props_open = false;
            ok_clicked = true;
        }
        ImGui::SetCursorScreenPos(ImVec2(ax - (bw + bgap), byy));
        if (Button("Cancel")) app.display_props_open = false;
        ImGui::SetCursorScreenPos(ImVec2(ax, byy));
        if (Button("Apply", ImVec2(0, 0), false, changed))
            app.display_props_original = app.display_props_pending;  // move revert baseline
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) app.display_props_open = false;
    }
    EndWindow95();

    // closed by anything other than OK (Cancel / Esc / the X) reverts the
    // live-previewed color back to what it was when the dialog opened
    if (!app.display_props_open && !ok_clicked)
        app.desktop_color = app.display_props_original;
}
