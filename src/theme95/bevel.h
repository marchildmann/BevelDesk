// Theme95 bevel primitives — the four Win95 3D edge conventions
// (light always from the top-left), drawn as 1px AddRectFilled lines.
#pragma once
#include "theme95/palette.h"

namespace t95 {

// 1px frame with distinct top-left / bottom-right colors.
void Edge(ImDrawList* dl, ImVec2 mn, ImVec2 mx, ImU32 tl, ImU32 br);
// 2px raised button bevel: outer TL white / BR black, inner TL 3DLight / BR shadow.
void BevelRaised(ImDrawList* dl, ImVec2 mn, ImVec2 mx, bool fill = true);
// 2px pressed button bevel (inverse) — label should shift +1,+1.
void BevelPressed(ImDrawList* dl, ImVec2 mn, ImVec2 mx, bool fill = true);
// 2px sunken field (edit/listview): outer TL shadow / BR white, inner TL black / BR 3DLight.
void SunkenField(ImDrawList* dl, ImVec2 mn, ImVec2 mx, ImU32 fill_col = WINBG);
// 2px window frame: outer TL 3DLight / BR black, inner TL white / BR shadow.
void WindowFrame(ImDrawList* dl, ImVec2 mn, ImVec2 mx, bool fill = true);
// 1px sunken groove (status bar wells, separators).
void ThinSunken(ImDrawList* dl, ImVec2 mn, ImVec2 mx);
// 1px raised (hover state of flat toolbar buttons).
void ThinRaised(ImDrawList* dl, ImVec2 mn, ImVec2 mx);
// 50% white-on-face checkerboard (scrollbar tracks, active task buttons).
void DitherFill(ImDrawList* dl, ImVec2 mn, ImVec2 mx);

} // namespace t95
