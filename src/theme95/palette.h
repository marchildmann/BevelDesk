// Theme95 palette + metrics. The chrome colors are reassignable globals so the
// desktop can switch schemes (Silver / NeXT Night) at runtime — every drawing
// call site reads these unchanged; ApplyScheme() swaps their values. See
// palette.cpp and PLAN_NEXT.md.
#pragma once
#include "imgui.h"

namespace t95 {

// ---- themeable chrome colors (defined in palette.cpp; default = Silver) -----
extern ImU32 FACE, LIGHT, HILIGHT, SHADOW, DKSHADOW;
extern ImU32 TITLE_ACT, TITLE_ACT2, TITLE_INACT, TITLE_INACT2, TITLE_TEXT;
extern ImU32 DESKTOP, WINBG, TEXT, GRAYTEXT, SEL, SEL_TEXT;

// ---- fixed colors: semantic, not chrome — never change per scheme ----------
constexpr ImU32 BLACK    = IM_COL32(0, 0, 0, 255);
constexpr ImU32 RED      = IM_COL32(255, 0, 0, 255);
constexpr ImU32 GREEN    = IM_COL32(0, 128, 0, 255);
constexpr ImU32 BLUE     = IM_COL32(0, 0, 255, 255);
constexpr ImU32 YELLOW   = IM_COL32(255, 255, 0, 255);
constexpr ImU32 NAVY     = IM_COL32(0, 0, 128, 255);
constexpr ImU32 DKYELLOW = IM_COL32(128, 128, 0, 255);

// ---- metrics (96 dpi Win95 defaults) ------------------------------------------
constexpr float TITLEBAR_H = 18.0f;
constexpr float WIN_BORDER = 4.0f;   // sizing border thickness
constexpr float TASKBAR_H  = 28.0f;
constexpr float CAPBTN_W   = 16.0f;
constexpr float CAPBTN_H   = 14.0f;

// ---- schemes ----------------------------------------------------------------
struct Palette {
    ImU32 face, light, hilight, shadow, dkshadow;
    ImU32 title_act, title_act2, title_inact, title_inact2, title_text;
    ImU32 desktop, winbg, text, graytext, sel, sel_text;
};
struct Style { bool chiseled = false; };   // NeXT-style black keyline (room to grow)

extern const Palette Palette95;     // Silver — the exact Win95 values
extern const Palette PaletteNight;  // charcoal dark mode
extern Style Sty;                   // active style flags

// Swap the active scheme: reassigns the color globals and re-applies ImGui style.
void ApplyScheme(const Palette& p, const Style& s);

} // namespace t95
