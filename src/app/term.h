// Minimal terminal emulator: a VT100/xterm subset good enough for an
// interactive shell (zsh/bash) — cursor movement, erase, SGR colors,
// scrollback, OSC window title, DA/DSR replies. Pure model, no ImGui.
#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

// VGA text-mode palette (ANSI order: 0-7 normal, 8-15 bright).
extern const unsigned char kVgaPalette[16][3];

struct TermCell {
    uint32_t ch = ' ';   // unicode codepoint
    uint8_t fg = 7;      // palette index
    uint8_t bg = 0;
};

class TermGrid {
public:
    int cols = 80, rows = 25;
    std::string title;                       // from OSC 0/2
    std::string reply;                       // pending responses -> write to pty
    bool cursor_visible = true;
    bool app_cursor_keys = false;            // DECCKM: arrows send ESC O A
    int cx = 0, cy = 0;
    std::deque<std::vector<TermCell>> scrollback; // lines pushed off the top
    size_t scrollback_max = 1000;

    void Init(int c, int r);
    void Feed(const char* data, size_t n);
    const TermCell* Row(int r) const { return &screen_[(size_t)r * cols]; }

private:
    std::vector<TermCell> screen_;
    uint8_t fg_ = 7, bg_ = 0;
    bool bold_ = false, rev_ = false;
    int state_ = 0;                          // 0 ground, 1 esc, 2 csi, 3 osc, 4 charset
    std::string csi_;
    std::string osc_;
    bool osc_esc_ = false;
    int save_cx_ = 0, save_cy_ = 0;
    uint32_t utf_cp_ = 0;
    int utf_rem_ = 0;

    TermCell Blank() const { return TermCell{' ', 7, bg_}; }
    TermCell& At(int y, int x) { return screen_[(size_t)y * cols + x]; }
    void PutChar(uint32_t cp);
    void CtrlChar(char c);
    void EscChar(char c);
    void CsiFinal(char f);
    void Sgr(const std::vector<int>& p);
    void LineFeed();
    void ScrollUp();
    void ClearScreen();
    void ClampCursor();
};
