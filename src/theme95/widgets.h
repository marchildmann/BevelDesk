// Theme95 interactive widgets — custom-drawn Win95 controls.
#pragma once
#include "theme95/palette.h"

namespace t95 {

// Bold UI font (Tahoma Bold etc.); null -> fake bold via 1px overdraw.
extern ImFont* FontBold;
// Monospace font (Monaco/Consolas/DejaVu Mono) for the MS-DOS Prompt;
// null -> default font (which is also monospaced).
extern ImFont* FontMono;
// Bold text (captions, Start button, task buttons).
void AddTextBold(ImDrawList* dl, ImVec2 pos, ImU32 col, const char* text,
                 const ImVec4* clip = nullptr);

// Classic push button. size {0,0} = auto from label (min 75x23, Win95 default).
bool Button(const char* label, ImVec2 size = ImVec2(0, 0), bool forced_pressed = false);

// Win95 radio button (sunken circle + dot). Returns true when clicked.
bool Radio(const char* label, bool selected);

// Flat COMCTL32-style toolbar button: flat, raised on hover, sunken when held.
// Glyph is drawn by callback at the button's top-left (button is w x h).
bool ToolButton(const char* id, ImVec2 size, void (*glyph)(ImDrawList*, ImVec2));

enum Glyph { GLYPH_CLOSE, GLYPH_MIN, GLYPH_MAX, GLYPH_RESTORE };
// 16x14 caption button at absolute screen pos.
bool CaptionButton(const char* id, ImVec2 pos, Glyph glyph);

// Win95 vertical scrollbar (16px wide): arrow buttons, dithered track,
// raised thumb. Occupies [mn,mx]; returns the new scroll offset
// (clamped to [0, content_h - view_h]).
float ScrollBarV(const char* id, ImVec2 mn, ImVec2 mx,
                 float view_h, float content_h, float scroll);

} // namespace t95
