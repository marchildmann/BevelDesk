// Procedural Win95-style icons drawn with ImDrawList (no image assets).
#pragma once
#include "imgui.h"

namespace icons95 {

// 32x32 desktop icons (p = top-left of the 32x32 cell)
void MyComputer32(ImDrawList* dl, ImVec2 p);
void RecycleBin32(ImDrawList* dl, ImVec2 p);
void Folder32(ImDrawList* dl, ImVec2 p);
void Document32(ImDrawList* dl, ImVec2 p);

// 14x14 small icons for list rows / title bars
void Folder14(ImDrawList* dl, ImVec2 p);
void Document14(ImDrawList* dl, ImVec2 p);
void MiniDos14(ImDrawList* dl, ImVec2 p);
void MiniComputer14(ImDrawList* dl, ImVec2 p);

// toolbar glyph: folder with an up arrow, drawn inside a 24x22 button
void UpFolderGlyph(ImDrawList* dl, ImVec2 button_tl);
// toolbar view-mode glyphs (24x22 buttons)
void LargeIconsGlyph(ImDrawList* dl, ImVec2 button_tl);
void DetailsGlyph(ImDrawList* dl, ImVec2 button_tl);

// BevelDesk mark (four beveled tiles, one teal), roughly 13x13, Start button
void BevelDeskLogo(ImDrawList* dl, ImVec2 p);

} // namespace icons95
