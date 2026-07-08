#include "app/term.h"
#include <cstdio>
#include <cstdlib>

const unsigned char kVgaPalette[16][3] = {
    { 0, 0, 0 },       { 170, 0, 0 },     { 0, 170, 0 },     { 170, 85, 0 },
    { 0, 0, 170 },     { 170, 0, 170 },   { 0, 170, 170 },   { 170, 170, 170 },
    { 85, 85, 85 },    { 255, 85, 85 },   { 85, 255, 85 },   { 255, 255, 85 },
    { 85, 85, 255 },   { 255, 85, 255 }, { 85, 255, 255 },  { 255, 255, 255 },
};

static uint8_t Nearest16(int r, int g, int b) {
    int best = 0;
    long best_d = -1;
    for (int i = 0; i < 16; ++i) {
        long dr = r - kVgaPalette[i][0], dg = g - kVgaPalette[i][1], db = b - kVgaPalette[i][2];
        long d = dr * dr + dg * dg + db * db;
        if (best_d < 0 || d < best_d) { best_d = d; best = i; }
    }
    return (uint8_t)best;
}

static uint8_t Map256(int n) {
    if (n < 0) return 7;
    if (n < 16) return (uint8_t)n;
    if (n < 232) {
        int q = n - 16;
        int r = (q / 36) % 6, g = (q / 6) % 6, b = q % 6;
        return Nearest16(r * 51, g * 51, b * 51);
    }
    int v = (n - 232) * 10 + 8;
    return Nearest16(v, v, v);
}

void TermGrid::Init(int c, int r) {
    cols = c;
    rows = r;
    screen_.assign((size_t)cols * rows, TermCell{});
    cx = cy = 0;
}

void TermGrid::ClampCursor() {
    if (cx < 0) cx = 0;
    if (cx > cols - 1) cx = cols - 1;
    if (cy < 0) cy = 0;
    if (cy > rows - 1) cy = rows - 1;
}

void TermGrid::LineFeed() {
    if (cy >= rows - 1) ScrollUp();
    else ++cy;
}

void TermGrid::ScrollUp() {
    scrollback.emplace_back(screen_.begin(), screen_.begin() + cols);
    while (scrollback.size() > scrollback_max) scrollback.pop_front();
    for (int y = 0; y < rows - 1; ++y)
        for (int x = 0; x < cols; ++x) At(y, x) = At(y + 1, x);
    for (int x = 0; x < cols; ++x) At(rows - 1, x) = Blank();
}

void TermGrid::ClearScreen() {
    for (auto& c : screen_) c = Blank();
}

void TermGrid::PutChar(uint32_t cp) {
    if (cx >= cols) { cx = 0; LineFeed(); }
    uint8_t f = fg_, b = bg_;
    if (bold_ && f < 8) f = (uint8_t)(f + 8);
    if (rev_) { uint8_t t = f; f = b; b = t; }
    At(cy, cx) = TermCell{cp, f, b};
    ++cx;
}

void TermGrid::CtrlChar(char c) {
    switch (c) {
    case '\r': cx = 0; break;
    case '\n': case 0x0B: case 0x0C: LineFeed(); break;
    case '\b': if (cx > 0) --cx; break;
    case '\t': cx = ((cx / 8) + 1) * 8; if (cx > cols - 1) cx = cols - 1; break;
    default: break; // BEL, SI, SO, ...
    }
}

void TermGrid::EscChar(char c) {
    state_ = 0;
    switch (c) {
    case '[': state_ = 2; csi_.clear(); break;
    case ']': state_ = 3; osc_.clear(); osc_esc_ = false; break;
    case '(': case ')': state_ = 4; break;         // charset select: eat next byte
    case '7': save_cx_ = cx; save_cy_ = cy; break;
    case '8': cx = save_cx_; cy = save_cy_; ClampCursor(); break;
    case 'M': if (cy > 0) --cy; break;              // reverse index (no scroll-down)
    case 'D': LineFeed(); break;
    case 'E': cx = 0; LineFeed(); break;
    case 'c': ClearScreen(); cx = cy = 0; fg_ = 7; bg_ = 0; bold_ = rev_ = false; break;
    case '=': case '>': break;                      // keypad modes
    default: break;
    }
}

void TermGrid::Sgr(const std::vector<int>& p) {
    if (p.empty()) { fg_ = 7; bg_ = 0; bold_ = rev_ = false; return; }
    for (size_t i = 0; i < p.size(); ++i) {
        int v = p[i];
        if (v == 0) { fg_ = 7; bg_ = 0; bold_ = rev_ = false; }
        else if (v == 1) bold_ = true;
        else if (v == 22) bold_ = false;
        else if (v == 7) rev_ = true;
        else if (v == 27) rev_ = false;
        else if (v >= 30 && v <= 37) fg_ = (uint8_t)(v - 30);
        else if (v == 39) fg_ = 7;
        else if (v >= 40 && v <= 47) bg_ = (uint8_t)(v - 40);
        else if (v == 49) bg_ = 0;
        else if (v >= 90 && v <= 97) fg_ = (uint8_t)(v - 90 + 8);
        else if (v >= 100 && v <= 107) bg_ = (uint8_t)(v - 100 + 8);
        else if ((v == 38 || v == 48) && i + 1 < p.size()) {
            uint8_t idx = 7;
            if (p[i + 1] == 5 && i + 2 < p.size()) { idx = Map256(p[i + 2]); i += 2; }
            else if (p[i + 1] == 2 && i + 4 < p.size()) {
                idx = Nearest16(p[i + 2], p[i + 3], p[i + 4]);
                i += 4;
            }
            if (v == 38) fg_ = idx; else bg_ = idx;
        }
    }
}

void TermGrid::CsiFinal(char f) {
    // split params; note a private prefix like '?' or '>'
    char prefix = 0;
    std::vector<int> p;
    {
        int cur = 0;
        bool has = false;
        for (char c : csi_) {
            if (c >= '0' && c <= '9') { cur = cur * 10 + (c - '0'); has = true; }
            else if (c == ';') { p.push_back(has ? cur : 0); cur = 0; has = false; }
            else if (!prefix && (c == '?' || c == '>' || c == '<' || c == '=')) prefix = c;
        }
        if (has || !p.empty()) p.push_back(has ? cur : 0);
    }
    auto P = [&](size_t i, int def) { return i < p.size() && p[i] > 0 ? p[i] : def; };

    switch (f) {
    case 'A': cy -= P(0, 1); ClampCursor(); break;
    case 'B': cy += P(0, 1); ClampCursor(); break;
    case 'C': cx += P(0, 1); ClampCursor(); break;
    case 'D': cx -= P(0, 1); ClampCursor(); break;
    case 'E': cx = 0; cy += P(0, 1); ClampCursor(); break;
    case 'F': cx = 0; cy -= P(0, 1); ClampCursor(); break;
    case 'G': cx = P(0, 1) - 1; ClampCursor(); break;
    case 'd': cy = P(0, 1) - 1; ClampCursor(); break;
    case 'H': case 'f': cy = P(0, 1) - 1; cx = P(1, 1) - 1; ClampCursor(); break;
    case 'J': {
        int m = p.empty() ? 0 : p[0];
        if (m == 0) {
            for (int x = cx; x < cols; ++x) At(cy, x) = Blank();
            for (int y = cy + 1; y < rows; ++y)
                for (int x = 0; x < cols; ++x) At(y, x) = Blank();
        } else if (m == 1) {
            for (int y = 0; y < cy; ++y)
                for (int x = 0; x < cols; ++x) At(y, x) = Blank();
            for (int x = 0; x <= cx && x < cols; ++x) At(cy, x) = Blank();
        } else {
            ClearScreen();
            if (m == 3) scrollback.clear();
        }
        break;
    }
    case 'K': {
        int m = p.empty() ? 0 : p[0];
        int x0 = (m == 0) ? cx : 0;
        int x1 = (m == 1) ? cx + 1 : cols;
        if (m == 2) { x0 = 0; x1 = cols; }
        for (int x = x0; x < x1 && x < cols; ++x) At(cy, x) = Blank();
        break;
    }
    case 'L': { // insert lines at cursor
        int n = P(0, 1);
        for (int k = 0; k < n; ++k) {
            for (int y = rows - 1; y > cy; --y)
                for (int x = 0; x < cols; ++x) At(y, x) = At(y - 1, x);
            for (int x = 0; x < cols; ++x) At(cy, x) = Blank();
        }
        break;
    }
    case 'M': { // delete lines at cursor
        int n = P(0, 1);
        for (int k = 0; k < n; ++k) {
            for (int y = cy; y < rows - 1; ++y)
                for (int x = 0; x < cols; ++x) At(y, x) = At(y + 1, x);
            for (int x = 0; x < cols; ++x) At(rows - 1, x) = Blank();
        }
        break;
    }
    case 'P': { // delete chars in line
        int n = P(0, 1);
        for (int x = cx; x < cols; ++x)
            At(cy, x) = (x + n < cols) ? At(cy, x + n) : Blank();
        break;
    }
    case '@': { // insert blank chars
        int n = P(0, 1);
        for (int x = cols - 1; x >= cx; --x)
            At(cy, x) = (x - n >= cx) ? At(cy, x - n) : Blank();
        break;
    }
    case 'X': { // erase chars
        int n = P(0, 1);
        for (int x = cx; x < cx + n && x < cols; ++x) At(cy, x) = Blank();
        break;
    }
    case 'm': if (!prefix) Sgr(p); break;
    case 'h': case 'l': {
        bool set = (f == 'h');
        if (prefix == '?') {
            for (int v : p) {
                if (v == 25) cursor_visible = set;
                else if (v == 1) app_cursor_keys = set;
                else if (v == 1049 || v == 47 || v == 1047) {
                    // alt screen enter/exit: we keep one screen — just clear
                    ClearScreen();
                    if (set) { cx = cy = 0; }
                }
            }
        }
        break;
    }
    case 's': save_cx_ = cx; save_cy_ = cy; break;
    case 'u': cx = save_cx_; cy = save_cy_; ClampCursor(); break;
    case 'c':
        if (prefix == '>') reply += "\x1b[>0;95;0c";     // secondary DA
        else reply += "\x1b[?1;2c";                       // primary DA: VT100+AVO
        break;
    case 'n': {
        int m = p.empty() ? 0 : p[0];
        if (m == 6) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "\x1b[%d;%dR", cy + 1, cx + 1);
            reply += buf;
        } else if (m == 5) reply += "\x1b[0n";
        break;
    }
    case 'r': case 't': default: break; // scroll region / window ops: ignored
    }
}

void TermGrid::Feed(const char* data, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        unsigned char b = (unsigned char)data[i];

        if (state_ == 2) { // CSI
            if (b >= 0x40 && b <= 0x7E) { CsiFinal((char)b); state_ = 0; }
            else if (csi_.size() < 64) csi_ += (char)b;
            continue;
        }
        if (state_ == 3) { // OSC ... BEL | ESC backslash
            if (b == 0x07 || (osc_esc_ && b == '\\')) {
                if (osc_esc_ && !osc_.empty()) osc_.pop_back(); // drop stored ESC
                if (osc_.size() >= 2 && (osc_[0] == '0' || osc_[0] == '2') && osc_[1] == ';')
                    title = osc_.substr(2);
                state_ = 0;
            } else {
                osc_esc_ = (b == 0x1B);
                if (osc_.size() < 512) osc_ += (char)b;
            }
            continue;
        }
        if (state_ == 4) { state_ = 0; continue; }   // charset byte
        if (state_ == 1) { EscChar((char)b); continue; }

        // ground
        if (utf_rem_ > 0) {
            if ((b & 0xC0) == 0x80) {
                utf_cp_ = (utf_cp_ << 6) | (b & 0x3F);
                if (--utf_rem_ == 0) PutChar(utf_cp_);
                continue;
            }
            utf_rem_ = 0; // malformed: fall through, reprocess as fresh byte
        }
        if (b == 0x1B) state_ = 1;
        else if (b < 0x20 || b == 0x7F) CtrlChar((char)b);
        else if (b < 0x80) PutChar(b);
        else if ((b & 0xE0) == 0xC0) { utf_cp_ = b & 0x1F; utf_rem_ = 1; }
        else if ((b & 0xF0) == 0xE0) { utf_cp_ = b & 0x0F; utf_rem_ = 2; }
        else if ((b & 0xF8) == 0xF0) { utf_cp_ = b & 0x07; utf_rem_ = 3; }
    }
}
