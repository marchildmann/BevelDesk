// Theme95 window chrome — frame, caption, drag, min/max/close.
#pragma once
#include "theme95/palette.h"

namespace t95 {

struct Chrome {
    const char* title;      // caption text
    const char* id;         // unique ImGui window id ("##explorer1")
    bool* p_open;                 // close button target
    bool* p_minimized = nullptr;  // minimize button target (null = none)
    bool* p_maximized = nullptr;  // maximize/restore target (null = none)
    // storage for the pre-maximize geometry (required when p_maximized is set)
    ImVec2* p_norm_pos = nullptr;
    ImVec2* p_norm_size = nullptr;
    bool request_focus;     // bring to front this frame
    bool dialog = false;    // dialog chrome: close button only, not resizable
    bool console = false;   // console window: system menu gains "Properties"
    ImVec2 def_pos, def_size;
    ImVec2 min_size = ImVec2(240, 160);
    ImVec2 max_size = ImVec2(1e9f, 1e9f); // set == min_size for fixed-size windows
    // outputs:
    bool focused = false;
    ImVec2 content_min, content_max; // client rect inside the chrome
};

// Build a Chrome for a centered, fixed-size modal dialog (close-only caption,
// not resizable). Fill in any extra fields on the result before BeginWindow95.
Chrome CenteredDialog(const char* title, const char* id, bool* p_open,
                      bool request_focus, ImVec2 size, float y_offset = -20.0f);

// Begins a Win95 window (frame + caption + buttons + drag/maximize +
// system menu on the caption icon).
// ALWAYS pair with EndWindow95(). Returns false if contents should be skipped.
bool BeginWindow95(Chrome& c);
void EndWindow95();

// QA hook: force the system menu open for the given window id next frame.
void DebugOpenSysMenu(const char* window_id);

} // namespace t95
