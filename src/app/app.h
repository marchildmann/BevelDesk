// Application model: desktop state + Explorer / MS-DOS Prompt windows.
#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include "imgui.h"
#include "app/term.h"
#include "platform/pty.h"

struct FileEntry {
    std::string name;
    std::string type;
    unsigned long long size = 0;
    bool is_dir = false;
};

struct ExplorerWin {
    int id = 0;
    std::string imgui_id;                 // "##explorer<N>" — stable ImGui identity
    std::filesystem::path path;
    std::string title;
    bool open = true;
    bool minimized = false;
    bool maximized = false;
    bool focused = false;
    bool request_focus = false;
    bool dirty = true;                    // rescan directory next frame
    int selected = -1;
    int view_mode = 0;                    // 0 = Details, 1 = Large Icons
    int sort_col = 0;                     // 0 name, 1 size, 2 type
    bool sort_asc = true;
    float scroll_y = 0;
    bool scroll_set = false;              // scrollbar moved -> push into child
    ImVec2 def_pos, def_size;
    ImVec2 norm_pos{0, 0}, norm_size{0, 0}; // pre-maximize geometry
    std::vector<FileEntry> entries;
    unsigned long long total_bytes = 0;

    void Navigate(const std::filesystem::path& p);
};

// One MS-DOS Prompt: a live shell on a pty + an 80x25 terminal grid.
struct DosWin {
    int id = 0;
    std::string imgui_id;                 // "##dos<N>"
    std::string title = "MS-DOS Prompt";
    bool open = true;
    bool minimized = false;
    bool focused = false;
    bool request_focus = false;
    float scroll_px = 0;                  // scrollback view offset (px)
    bool stick_bottom = true;             // follow output
    ImVec2 def_pos;
    Pty pty;
    TermGrid term;
};

// One tab in the terminal drawer: a shell on a pty + its terminal grid.
struct DrawerTab {
    int id = 0;
    Pty pty;
    TermGrid term;
    int cols = 0, rows = 0;   // current grid size (detect resize)
    bool spawned = false;
};

struct AppState {
    std::vector<ExplorerWin> wins;
    std::vector<std::unique_ptr<DosWin>> dos_wins;
    int next_id = 1;
    int active_win_id = 0;                // last-focused explorer (taskbar pressed state)
    bool start_open = false;
    bool start_opened_this_frame = false;
    bool force_start_open = false;        // --demo start (screenshot testing)
    bool quit_requested = false;
    int desktop_selected = -1;            // 0 = My Computer, 1 = Recycle Bin

    // UI zoom (Ctrl/Cmd + wheel, or +/-/0). 1.92 dynamic fonts rebake crisply.
    float zoom = 1.0f;

    // Terminal drawer — BevelDesk original: live shells docked below the
    // taskbar (the taskbar becomes its handle/lid). Toggle with the button
    // next to Start or Ctrl+`; drag the taskbar edge to resize; multiple
    // tabbed sessions. Persists (hidden, not killed) across toggles.
    bool drawer_open = false;
    bool drawer_focus_request = false;
    float drawer_height = 240.0f;
    std::vector<std::unique_ptr<DrawerTab>> drawer_tabs;
    int drawer_active = 0;
    int drawer_next_id = 1;

    // desktop appearance
    ImU32 desktop_color = IM_COL32(0, 128, 128, 255);   // the teal, changeable
    bool display_props_open = false;
    bool display_props_opened_now = false;
    ImU32 display_props_pending = IM_COL32(0, 128, 128, 255);
    ImU32 display_props_original = IM_COL32(0, 128, 128, 255); // for Cancel revert

    // About dialog
    bool about_open = false;
    bool about_opened_now = false;

    // Run dialog
    bool run_open = false;
    bool run_opened_now = false;
    char run_buf[512] = {0};

    // Shut Down dialog
    bool shutdown_open = false;
    bool shutdown_opened_this_frame = false;
    int shutdown_choice = 0;              // 0 = shut down, 1 = restart
    ImTextureID checker_tex = (ImTextureID)0; // 2x2 dither for the screen fade

    void OpenExplorer(const std::filesystem::path& p);
};

extern AppState g_app;
