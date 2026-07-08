// Theme95 palette + metrics — the Windows 95 "Windows Standard" scheme.
#pragma once
#include "imgui.h"

namespace t95 {

// ---- system colors ----------------------------------------------------------
constexpr ImU32 FACE        = IM_COL32(192, 192, 192, 255); // ButtonFace
constexpr ImU32 LIGHT       = IM_COL32(223, 223, 223, 255); // 3DLight
constexpr ImU32 HILIGHT     = IM_COL32(255, 255, 255, 255); // ButtonHighlight
constexpr ImU32 SHADOW      = IM_COL32(128, 128, 128, 255); // ButtonShadow
constexpr ImU32 DKSHADOW    = IM_COL32(0, 0, 0, 255);       // ButtonDkShadow
constexpr ImU32 BLACK       = IM_COL32(0, 0, 0, 255);
constexpr ImU32 TITLE_ACT    = IM_COL32(0, 0, 128, 255);     // ActiveTitle (navy)
constexpr ImU32 TITLE_ACT2   = IM_COL32(16, 132, 208, 255);  // GradientActiveTitle end
constexpr ImU32 TITLE_INACT  = IM_COL32(128, 128, 128, 255); // InactiveTitle
constexpr ImU32 TITLE_INACT2 = IM_COL32(181, 181, 181, 255); // GradientInactiveTitle end
constexpr ImU32 TITLE_TEXT   = IM_COL32(255, 255, 255, 255);
constexpr ImU32 DESKTOP     = IM_COL32(0, 128, 128, 255);   // teal
constexpr ImU32 WINBG       = IM_COL32(255, 255, 255, 255); // Window
constexpr ImU32 TEXT        = IM_COL32(0, 0, 0, 255);
constexpr ImU32 GRAYTEXT    = IM_COL32(128, 128, 128, 255);
constexpr ImU32 SEL         = IM_COL32(0, 0, 128, 255);     // Highlight
constexpr ImU32 SEL_TEXT    = IM_COL32(255, 255, 255, 255);

// ---- VGA accents for icons / logo --------------------------------------------
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

} // namespace t95
