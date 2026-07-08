#include "theme95/window.h"
#include "theme95/bevel.h"
#include "theme95/widgets.h"
#include "theme95/icons95.h"
#include "imgui_internal.h"
#include <cfloat>
#include <string>

namespace t95 {

// ---- system menu state (one menu open at a time, keyed by window id) --------
static std::string g_sysmenu_owner;
static bool g_sysmenu_opened_now = false;

void DebugOpenSysMenu(const char* window_id) {
    g_sysmenu_owner = window_id;
    g_sysmenu_opened_now = true;
}

bool BeginWindow95(Chrome& c) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    bool maximized = c.p_maximized && *c.p_maximized;

    if (maximized) {
        ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(vp->Size.x, vp->Size.y - TASKBAR_H), ImGuiCond_Always);
    } else {
        ImGui::SetNextWindowPos(c.def_pos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(c.def_size, ImGuiCond_FirstUseEver);
    }
    ImGui::SetNextWindowSizeConstraints(c.min_size, c.max_size);
    if (c.request_focus) ImGui::SetNextWindowFocus();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(FACE));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (maximized) flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    if (c.dialog) flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    bool vis = ImGui::Begin(c.id, nullptr, flags);
    if (!vis) return false;

    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImVec2 pmax(pos.x + size.x, pos.y + size.y);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    WindowFrame(dl, pos, pmax);

    bool sysmenu_open = !c.dialog && g_sysmenu_owner == c.id;
    // caption stays "active" while its system menu is open, like Win95
    c.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) || sysmenu_open;

    const float B = WIN_BORDER;
    ImVec2 tmin(pos.x + B, pos.y + B);
    ImVec2 tmax(pmax.x - B, pos.y + B + TITLEBAR_H);
    // horizontal caption gradient (dark at the left, bright at the right)
    ImU32 g0 = c.focused ? TITLE_ACT : TITLE_INACT;
    ImU32 g1 = c.focused ? TITLE_ACT2 : TITLE_INACT2;
    dl->AddRectFilledMultiColor(tmin, tmax, g0, g1, g1, g0);

    // mini icon + bold title text (clipped before the buttons)
    float text_x = tmin.x + 4;
    if (!c.dialog) {
        icons95::MiniComputer14(dl, ImVec2(tmin.x + 2, tmin.y + 2));
        text_x = tmin.x + 20;
    }
    float btn_row_x = c.dialog ? (tmax.x - 2 - CAPBTN_W - 2)
                               : (tmax.x - 2 - CAPBTN_W * 3 - 2); // [min][max] gap [close]
    ImVec4 clip(text_x, tmin.y, btn_row_x - 2, tmax.y);
    float text_y = tmin.y + (TITLEBAR_H - ImGui::GetFontSize()) * 0.5f;
    AddTextBold(dl, ImVec2(text_x, text_y), TITLE_TEXT, c.title, &clip);

    // system-menu box (the caption icon): click opens the system menu,
    // double-click closes the window, like Win95
    float drag_x = tmin.x;
    if (!c.dialog) {
        ImGui::SetCursorScreenPos(tmin);
        ImGui::InvisibleButton("##sysmenu", ImVec2(18, TITLEBAR_H));
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            if (c.p_open) *c.p_open = false;
            g_sysmenu_owner.clear();
            sysmenu_open = false;
        } else if (ImGui::IsItemClicked(0)) {
            if (sysmenu_open) g_sysmenu_owner.clear();
            else { g_sysmenu_owner = c.id; g_sysmenu_opened_now = true; }
            sysmenu_open = g_sysmenu_owner == c.id;
        }
        drag_x = tmin.x + 18;
    }

    // maximize <-> restore, keeping the pre-maximize geometry
    auto toggle_maximize = [&]() {
        if (!*c.p_maximized) {
            if (c.p_norm_pos) *c.p_norm_pos = pos;
            if (c.p_norm_size) *c.p_norm_size = size;
            *c.p_maximized = true;
        } else {
            *c.p_maximized = false;
            if (c.p_norm_pos && c.p_norm_size && c.p_norm_size->x > 0) {
                ImGui::SetWindowPos(*c.p_norm_pos);
                ImGui::SetWindowSize(*c.p_norm_size);
            }
        }
    };

    // drag zone (also: double-click caption = maximize toggle)
    ImGui::SetCursorScreenPos(ImVec2(drag_x, tmin.y));
    float drag_w = btn_row_x - drag_x;
    if (drag_w > 8) {
        ImGui::InvisibleButton("##caption_drag", ImVec2(drag_w, TITLEBAR_H));
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && c.p_maximized &&
            !c.dialog)
            toggle_maximize();
        else if (ImGui::IsItemActive() && !maximized) {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::SetWindowPos(ImVec2(pos.x + io.MouseDelta.x, pos.y + io.MouseDelta.y));
        }
    }

    // caption buttons, inset 2px from top/right of the caption
    float by = tmin.y + 2;
    ImVec2 close_p(tmax.x - 2 - CAPBTN_W, by);
    if (!c.dialog) {
        ImVec2 max_p(close_p.x - 2 - CAPBTN_W, by);
        // without a maximize button, minimize sits right next to close
        ImVec2 min_p(c.p_maximized ? max_p.x - CAPBTN_W : max_p.x, by);
        if (CaptionButton("##cap_min", min_p, GLYPH_MIN) && c.p_minimized)
            *c.p_minimized = true;
        if (c.p_maximized &&
            CaptionButton("##cap_max", max_p, maximized ? GLYPH_RESTORE : GLYPH_MAX))
            toggle_maximize();
    }
    if (CaptionButton("##cap_close", close_p, GLYPH_CLOSE) && c.p_open)
        *c.p_open = false;

    // ---- system menu popup (Restore/Move/Size/Minimize/Maximize/Close) ----
    if (sysmenu_open) {
        struct MenuIt { const char* label; const char* shortcut; bool enabled; char act; bool sep_before; };
        MenuIt items[8];
        int n_items = 0;
        int n_seps = 0;
        items[n_items++] = { "Restore", nullptr, maximized, 'r', false };
        items[n_items++] = { "Move", nullptr, false, 0, false };     // keyboard move: not implemented
        items[n_items++] = { "Size", nullptr, false, 0, false };     // keyboard size: not implemented
        items[n_items++] = { "Minimize", nullptr, c.p_minimized != nullptr, 'n', false };
        items[n_items++] = { "Maximize", nullptr, c.p_maximized != nullptr && !maximized, 'x', false };
        items[n_items++] = { "Close", "Alt+F4", true, 'c', true };
        n_seps = 1;
        if (c.console) {
            items[n_items++] = { "Properties", nullptr, false, 0, true }; // listed, like Win95 consoles
            n_seps = 2;
        }
        const float mw = 150, ih = 18;
        const float mh = 6 + n_items * ih + n_seps * 5;
        ImGui::SetNextWindowPos(ImVec2(tmin.x, tmax.y));
        ImGui::SetNextWindowSize(ImVec2(mw, mh));
        if (g_sysmenu_opened_now) ImGui::SetNextWindowFocus();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(FACE));
        ImGui::Begin("##sysmenu95", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoSavedSettings);
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImDrawList* mdl = ImGui::GetWindowDrawList();
        ImVec2 mn = ImGui::GetWindowPos();
        ImVec2 mx(mn.x + mw, mn.y + mh);
        BevelRaised(mdl, mn, mx);

        float y = mn.y + 3;
        for (int ii = 0; ii < n_items; ++ii) {
            const MenuIt& it = items[ii];
            if (it.sep_before) {
                ThinSunken(mdl, ImVec2(mn.x + 4, y + 1), ImVec2(mx.x - 4, y + 3));
                y += 5;
            }
            ImVec2 imin(mn.x + 3, y), imax(mx.x - 3, y + ih);
            ImGui::SetCursorScreenPos(imin);
            ImGui::PushID(it.label);
            bool item_clicked = ImGui::InvisibleButton("##it", ImVec2(imax.x - imin.x, ih));
            ImGui::PopID();
            bool hov = it.enabled && ImGui::IsItemHovered();
            if (hov) mdl->AddRectFilled(imin, imax, SEL);
            float ty = y + (ih - ImGui::GetFontSize()) * 0.5f;
            if (!it.enabled) {
                // Win95 disabled text: gray with a white emboss
                mdl->AddText(ImVec2(imin.x + 9, ty + 1), HILIGHT, it.label);
                mdl->AddText(ImVec2(imin.x + 8, ty), GRAYTEXT, it.label);
            } else {
                mdl->AddText(ImVec2(imin.x + 8, ty), hov ? SEL_TEXT : TEXT, it.label);
            }
            if (it.shortcut) {
                ImVec2 sts = ImGui::CalcTextSize(it.shortcut);
                ImU32 scol = !it.enabled ? GRAYTEXT : (hov ? SEL_TEXT : TEXT);
                mdl->AddText(ImVec2(imax.x - 8 - sts.x, ty), scol, it.shortcut);
            }
            if (item_clicked && it.enabled) {
                switch (it.act) {
                case 'r': case 'x': toggle_maximize(); break;
                case 'n': *c.p_minimized = true; break;
                case 'c': if (c.p_open) *c.p_open = false; break;
                }
                g_sysmenu_owner.clear();
            }
            y += ih;
        }

        if (!g_sysmenu_opened_now && !ImGui::IsWindowFocused())
            g_sysmenu_owner.clear();
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            g_sysmenu_owner.clear();
        g_sysmenu_opened_now = false;

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    c.content_min = ImVec2(pos.x + B, tmax.y + 1);
    c.content_max = ImVec2(pmax.x - B, pmax.y - B);
    // Declare the full client extent up front with a Dummy. Our content is
    // drawn with absolute SetCursorScreenPos + DrawList; without this, 1.92
    // warns that later cursor moves "extend window boundaries" without an item.
    ImGui::SetCursorScreenPos(c.content_min);
    ImGui::Dummy(ImVec2(c.content_max.x - c.content_min.x,
                        c.content_max.y - c.content_min.y));
    ImGui::SetCursorScreenPos(c.content_min);
    return true;
}

void EndWindow95() {
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

} // namespace t95
