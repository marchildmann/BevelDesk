#include "shell/explorer.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include "theme95/icons95.h"
#include <algorithm>
#include <cctype>
#include <cstdio>

namespace fs = std::filesystem;
using namespace t95;

std::string FormatKB(unsigned long long bytes) {
    unsigned long long kb = (bytes + 1023) / 1024;
    std::string digits = std::to_string(kb);
    std::string out;
    int c = 0;
    for (int i = (int)digits.size() - 1; i >= 0; --i) {
        out.insert(out.begin(), digits[i]);
        if (++c % 3 == 0 && i > 0) out.insert(out.begin(), ',');
    }
    return out + "KB";
}

static std::string TypeFor(const fs::directory_entry& e, bool is_dir) {
    if (is_dir) return "File Folder";
    std::string ext = e.path().extension().string();
    if (ext.size() > 1) {
        ext.erase(0, 1);
        for (char& ch : ext) ch = (char)std::toupper((unsigned char)ch);
        if (ext.size() > 8) ext.resize(8);
        return ext + " File";
    }
    return "File";
}

static int CmpCI(const std::string& a, const std::string& b) {
    size_t n = a.size() < b.size() ? a.size() : b.size();
    for (size_t i = 0; i < n; ++i) {
        int ca = std::tolower((unsigned char)a[i]);
        int cb = std::tolower((unsigned char)b[i]);
        if (ca != cb) return ca < cb ? -1 : 1;
    }
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    return 0;
}

void SortEntries(ExplorerWin& w) {
    std::stable_sort(w.entries.begin(), w.entries.end(),
                     [&w](const FileEntry& a, const FileEntry& b) {
        if (a.is_dir != b.is_dir) return a.is_dir;      // folders always first
        int r = 0;
        switch (w.sort_col) {
        case 1: r = (a.size < b.size) ? -1 : (a.size > b.size) ? 1 : 0; break;
        case 2: r = CmpCI(a.type, b.type); break;
        default: break;
        }
        if (r == 0) r = CmpCI(a.name, b.name);
        return w.sort_asc ? (r < 0) : (r > 0);
    });
    w.selected = -1;                                    // indices shifted
}

static void Refresh(ExplorerWin& w) {
    w.entries.clear();
    w.total_bytes = 0;
    w.dirty = false;
    std::error_code ec;
    fs::directory_iterator it(w.path, fs::directory_options::skip_permission_denied, ec);
    if (ec) return;
    for (const auto& e : it) {
        std::string name = e.path().filename().string();
        if (name.empty() || name[0] == '.') continue;   // Win95 default: hide hidden
        std::error_code ec2;
        bool is_dir = e.is_directory(ec2);
        FileEntry fe;
        fe.name = name;
        fe.is_dir = is_dir;
        if (!is_dir) {
            fe.size = e.file_size(ec2);
            if (ec2) fe.size = 0;
            w.total_bytes += fe.size;
        }
        fe.type = TypeFor(e, is_dir);
        w.entries.push_back(std::move(fe));
        if (w.entries.size() >= 2000) break;            // sanity cap for huge dirs
    }
    SortEntries(w);
}

// Decorative Win95 menu bar (File Edit View Help) with hover highlight.
static void MenuBar(ImDrawList* dl, ImVec2 mn, float width) {
    static const char* items[] = { "File", "Edit", "View", "Help" };
    float x = mn.x + 6;
    for (const char* item : items) {
        ImVec2 ts = ImGui::CalcTextSize(item);
        ImVec2 bmin(x - 4, mn.y + 1), bmax(x + ts.x + 4, mn.y + 17);
        ImGui::SetCursorScreenPos(bmin);
        ImGui::InvisibleButton(item, ImVec2(bmax.x - bmin.x, bmax.y - bmin.y));
        bool hov = ImGui::IsItemHovered();
        if (hov) dl->AddRectFilled(bmin, bmax, SEL);
        dl->AddText(ImVec2(x, mn.y + 3), hov ? SEL_TEXT : TEXT, item);
        x += ts.x + 14;
    }
    (void)width;
}

// Details listview (Name/Size/Type, sortable headers, custom scrollbar).
// Draws inside the already-painted sunken field [lmin,lmax]. Returns the entry
// index to navigate into (double-clicked folder), or -1.
static int DrawDetailsView(ExplorerWin& w, ImDrawList* dl,
                           ImVec2 lmin, ImVec2 lmax, float inner_w) {
    int navigate_to = -1;
    const float row_h = 17;
    float hx = lmin.x + 2, hy = lmin.y + 2, hh = 16;
    float view_h = lmax.y - 2 - (hy + hh);
    float content_h = row_h * (float)w.entries.size();
    bool need_sb = content_h > view_h;
    float sb_w = need_sb ? 16.0f : 0.0f;
    float avail_w = inner_w - sb_w;

    float col_size = 84, col_type = 96;
    float col_name = avail_w - col_size - col_type;
    if (col_name < 120) { col_name = 120; }

    // header row: raised gray cells, clickable to sort
    struct { const char* label; float wdt; } cols[] = {
        { "Name", col_name }, { "Size", col_size }, { "Type", col_type } };
    float cx = hx;
    for (int ci = 0; ci < 3; ++ci) {
        bool right_align = (ci == 1);          // Size: LVCFMT_RIGHT
        float cw2 = (ci == 2) ? (lmax.x - 2 - cx) : cols[ci].wdt;
        if (cw2 < 20) cw2 = 20;
        ImGui::SetCursorScreenPos(ImVec2(cx, hy));
        ImGui::PushID(ci);
        bool clicked = ImGui::InvisibleButton("##hdr", ImVec2(cw2, hh));
        ImGui::PopID();
        bool held = ImGui::IsItemActive() && ImGui::IsItemHovered();
        if (held) BevelPressed(dl, ImVec2(cx, hy), ImVec2(cx + cw2, hy + hh));
        else      BevelRaised(dl, ImVec2(cx, hy), ImVec2(cx + cw2, hy + hh));
        float off = held ? 1.0f : 0.0f;
        ImVec2 lts = ImGui::CalcTextSize(cols[ci].label);
        float lx = right_align ? (cx + cw2 - 8 - lts.x) : (cx + 5);
        ImVec4 clip(cx + 4, hy, cx + cw2 - 4, hy + hh);
        dl->AddText(nullptr, 0.0f, ImVec2(lx + off, hy + 1 + off), TEXT,
                    cols[ci].label, nullptr, 0.0f, &clip);
        if (clicked) {
            if (w.sort_col == ci) w.sort_asc = !w.sort_asc;
            else { w.sort_col = ci; w.sort_asc = true; }
            SortEntries(w);
        }
        cx += cw2;
    }

    // rows (scrolling child + custom Win95 scrollbar)
    ImGui::SetCursorScreenPos(ImVec2(hx, hy + hh));
    std::string child_id = "list:" + w.path.string();
    ImGui::BeginChild(child_id.c_str(), ImVec2(inner_w - sb_w, view_h),
                      ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
    // SetScrollY only applies at next frame's Begin — while pushing,
    // GetScrollY still returns the old value and must not overwrite
    // w.scroll_y (that ate every other drag delta: thumb at half speed)
    bool scroll_pushed = w.scroll_set;
    if (w.scroll_set) { ImGui::SetScrollY(w.scroll_y); w.scroll_set = false; }
    ImDrawList* cdl = ImGui::GetWindowDrawList();
    // rows must stack at exactly row_h (Win95 density) — the global
    // ItemSpacing.y would add 4px/row and desync all scrollbar math
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    for (int i = 0; i < (int)w.entries.size(); ++i) {
        const FileEntry& fe = w.entries[i];
        ImVec2 rp = ImGui::GetCursorScreenPos();
        ImGui::PushID(i);
        ImGui::InvisibleButton("##row", ImVec2(inner_w - sb_w - 2, row_h));
        ImGui::PopID();
        bool hovered = ImGui::IsItemHovered();
        if (hovered && ImGui::IsMouseClicked(0)) w.selected = i;
        if (hovered && ImGui::IsMouseDoubleClicked(0) && fe.is_dir) navigate_to = i;

        // icon + name (only the name cell highlights, Win95-style)
        if (fe.is_dir) icons95::Folder14(cdl, ImVec2(rp.x + 2, rp.y + 1));
        else           icons95::Document14(cdl, ImVec2(rp.x + 2, rp.y + 1));
        ImVec2 ts = ImGui::CalcTextSize(fe.name.c_str());
        bool sel = (w.selected == i);
        float name_max = rp.x + col_name - 4;
        if (sel) {
            float bg_r = rp.x + 20 + ts.x + 2;
            if (bg_r > name_max) bg_r = name_max;
            cdl->AddRectFilled(ImVec2(rp.x + 18, rp.y), ImVec2(bg_r, rp.y + row_h - 1), SEL);
        }
        ImVec4 nclip(rp.x + 20, rp.y, name_max, rp.y + row_h);
        cdl->AddText(nullptr, 0.0f, ImVec2(rp.x + 20, rp.y + 2),
                     sel ? SEL_TEXT : TEXT, fe.name.c_str(), nullptr, 0.0f, &nclip);

        // size (right-aligned) + type
        if (!fe.is_dir) {
            std::string sz = FormatKB(fe.size);
            ImVec2 sts = ImGui::CalcTextSize(sz.c_str());
            cdl->AddText(ImVec2(rp.x + col_name + col_size - 8 - sts.x, rp.y + 2), TEXT, sz.c_str());
        }
        ImVec4 tclip(rp.x + col_name + col_size, rp.y, rp.x + inner_w - sb_w - 4, rp.y + row_h);
        cdl->AddText(nullptr, 0.0f, ImVec2(rp.x + col_name + col_size + 4, rp.y + 2),
                     TEXT, fe.type.c_str(), nullptr, 0.0f, &tclip);
    }
    ImGui::PopStyleVar();
    if (!scroll_pushed) w.scroll_y = ImGui::GetScrollY();
    ImGui::EndChild();
    if (need_sb) {
        float ns = ScrollBarV("##vsb", ImVec2(lmax.x - 2 - 16, hy + hh),
                              ImVec2(lmax.x - 2, lmax.y - 2),
                              view_h, content_h, w.scroll_y);
        if (ns != w.scroll_y) { w.scroll_y = ns; w.scroll_set = true; }
    }
    return navigate_to;
}

// Large Icons view — a flowing 32px-icon grid. Same field/return contract.
static int DrawIconsView(ExplorerWin& w, ImDrawList* dl,
                         ImVec2 lmin, ImVec2 lmax, float inner_w) {
    int navigate_to = -1;
    const float cell_w = 76, cell_h = 66;
    float gy = lmin.y + 2;
    float view_h = lmax.y - 2 - gy;
    int n = (int)w.entries.size();
    int per_row = (int)((inner_w - 16) / cell_w);   // worst case with scrollbar
    if (per_row < 1) per_row = 1;
    int rows = (n + per_row - 1) / per_row;
    float content_h = rows * cell_h + 4;
    bool need_sb = content_h > view_h;
    float sb_w = need_sb ? 16.0f : 0.0f;
    per_row = (int)((inner_w - sb_w) / cell_w);
    if (per_row < 1) per_row = 1;
    rows = (n + per_row - 1) / per_row;
    content_h = rows * cell_h + 4;

    ImGui::SetCursorScreenPos(ImVec2(lmin.x + 2, gy));
    std::string child_id = "icons:" + w.path.string();
    ImGui::BeginChild(child_id.c_str(), ImVec2(inner_w - sb_w, view_h),
                      ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
    bool scroll_pushed = w.scroll_set;   // see details view: avoid stale readback
    if (w.scroll_set) { ImGui::SetScrollY(w.scroll_y); w.scroll_set = false; }
    ImDrawList* cdl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();    // scroll-adjusted
    ImGui::Dummy(ImVec2(1, content_h));             // scroll extent
    for (int i = 0; i < n; ++i) {
        const FileEntry& fe = w.entries[i];
        int r = i / per_row, cidx = i % per_row;
        ImVec2 cp(origin.x + cidx * cell_w, origin.y + r * cell_h);
        ImGui::SetCursorScreenPos(cp);
        ImGui::PushID(i);
        ImGui::InvisibleButton("##cell", ImVec2(cell_w, cell_h));
        ImGui::PopID();
        bool hovered = ImGui::IsItemHovered();
        if (hovered && ImGui::IsMouseClicked(0)) w.selected = i;
        if (hovered && ImGui::IsMouseDoubleClicked(0) && fe.is_dir) navigate_to = i;

        ImVec2 ip(cp.x + (cell_w - 32) * 0.5f, cp.y + 3);
        if (fe.is_dir) icons95::Folder32(cdl, ip);
        else           icons95::Document32(cdl, ip);

        bool sel = (w.selected == i);
        ImVec2 ts = ImGui::CalcTextSize(fe.name.c_str());
        float tw = ts.x < cell_w - 6 ? ts.x : cell_w - 6;
        ImVec2 tp(cp.x + (cell_w - tw) * 0.5f, cp.y + 39);
        if (sel)
            cdl->AddRectFilled(ImVec2(tp.x - 2, tp.y - 1),
                               ImVec2(tp.x + tw + 2, tp.y + ts.y + 1), SEL);
        ImVec4 tclip(tp.x, tp.y, tp.x + tw, tp.y + ts.y);
        cdl->AddText(nullptr, 0.0f, tp, sel ? SEL_TEXT : TEXT,
                     fe.name.c_str(), nullptr, 0.0f, &tclip);
    }
    if (!scroll_pushed) w.scroll_y = ImGui::GetScrollY();
    ImGui::EndChild();
    if (need_sb) {
        float ns = ScrollBarV("##vsb", ImVec2(lmax.x - 2 - 16, gy),
                              ImVec2(lmax.x - 2, lmax.y - 2),
                              view_h, content_h, w.scroll_y);
        if (ns != w.scroll_y) { w.scroll_y = ns; w.scroll_set = true; }
    }
    return navigate_to;
}

static void DrawExplorer(AppState& app, ExplorerWin& w) {
    if (w.dirty) Refresh(w);

    Chrome c;
    c.title = w.title.c_str();
    c.id = w.imgui_id.c_str();
    c.p_open = &w.open;
    c.p_minimized = &w.minimized;
    c.p_maximized = &w.maximized;
    c.p_norm_pos = &w.norm_pos;
    c.p_norm_size = &w.norm_size;
    c.request_focus = w.request_focus > 0 && !app.ModalOpen();
    c.def_pos = w.def_pos;
    c.def_size = w.def_size;
    if (w.request_focus > 0) --w.request_focus;

    bool vis = BeginWindow95(c);
    if (!vis) { EndWindow95(); return; }
    w.focused = c.focused;
    if (w.focused) app.active_win_id = w.id;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cmin = c.content_min, cmax = c.content_max;
    float cw = cmax.x - cmin.x;

    // ---- menu bar (18px) ----
    MenuBar(dl, cmin, cw);
    float y = cmin.y + 18;
    ThinSunken(dl, ImVec2(cmin.x + 1, y), ImVec2(cmax.x - 1, y + 2));
    y += 3;

    // ---- toolbar: Up button + address well (26px) ----
    ImGui::SetCursorScreenPos(ImVec2(cmin.x + 2, y + 1));
    if (ToolButton("##up", ImVec2(24, 22), icons95::UpFolderGlyph)) {
        fs::path parent = w.path.parent_path();
        if (!parent.empty() && parent != w.path) w.Navigate(parent);
    }
    // view-mode buttons at the toolbar's right end
    ImGui::SetCursorScreenPos(ImVec2(cmax.x - 2 - 48, y + 1));
    if (ToolButton("##viewicons", ImVec2(24, 22), icons95::LargeIconsGlyph))
        w.view_mode = 1;
    ImGui::SetCursorScreenPos(ImVec2(cmax.x - 2 - 24, y + 1));
    if (ToolButton("##viewdetails", ImVec2(24, 22), icons95::DetailsGlyph))
        w.view_mode = 0;

    ImVec2 amin(cmin.x + 30, y + 1), amax(cmax.x - 2 - 52, y + 23);
    SunkenField(dl, amin, amax, WINBG);
    icons95::Folder14(dl, ImVec2(amin.x + 4, amin.y + 4));
    {
        std::string pstr = w.path.string();
        ImVec4 clip(amin.x + 22, amin.y, amax.x - 20, amax.y);
        dl->AddText(nullptr, 0.0f, ImVec2(amin.x + 22, amin.y + 4), TEXT,
                    pstr.c_str(), nullptr, 0.0f, &clip);
        // decorative combo dropdown arrow (Win95 drive/path combo)
        ImVec2 dmin(amax.x - 18, amin.y + 2), dmax(amax.x - 2, amax.y - 2);
        BevelRaised(dl, dmin, dmax);
        float axc = (dmin.x + dmax.x) * 0.5f, ayc = (dmin.y + dmax.y) * 0.5f;
        dl->AddTriangleFilled(ImVec2(axc - 4, ayc - 2), ImVec2(axc + 4, ayc - 2),
                              ImVec2(axc, ayc + 2), BLACK);
    }
    y += 26;

    // ---- status bar geometry (bottom, 20px) ----
    float status_h = 20;
    ImVec2 smin(cmin.x, cmax.y - status_h), smax(cmax.x, cmax.y);

    // ---- listview: sunken white field (Details or Large Icons) ----
    ImVec2 lmin(cmin.x + 2, y + 1), lmax(cmax.x - 2, smin.y - 2);
    if (lmax.y > lmin.y + 30 && lmax.x > lmin.x + 60) {
        SunkenField(dl, lmin, lmax, WINBG);
        float inner_w = (lmax.x - lmin.x) - 4;
        int navigate_to = (w.view_mode == 0)
                              ? DrawDetailsView(w, dl, lmin, lmax, inner_w)
                              : DrawIconsView(w, dl, lmin, lmax, inner_w);
        if (navigate_to >= 0)
            w.Navigate(w.path / w.entries[navigate_to].name);
    }

    // ---- status bar: two sunken wells + resize grip ----
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d object(s)", (int)w.entries.size());
        float right_w = 110;
        ImVec2 amin2(smin.x + 1, smin.y + 2), amax2(smax.x - right_w - 4, smax.y - 1);
        ImVec2 bmin2(smax.x - right_w - 2, smin.y + 2), bmax2(smax.x - 14, smax.y - 1);
        ThinSunken(dl, amin2, amax2);
        ThinSunken(dl, bmin2, bmax2);
        dl->AddText(ImVec2(amin2.x + 5, amin2.y + 2), TEXT, buf);
        std::string total = FormatKB(w.total_bytes);
        dl->AddText(ImVec2(bmin2.x + 5, bmin2.y + 2), TEXT, total.c_str());
        // dotted resize grip
        for (int k = 0; k < 3; ++k) {
            float o = 4.0f * k;
            dl->AddLine(ImVec2(smax.x - 3 - o, smax.y - 2), ImVec2(smax.x - 2, smax.y - 3 - o), HILIGHT);
            dl->AddLine(ImVec2(smax.x - 4 - o, smax.y - 2), ImVec2(smax.x - 2, smax.y - 4 - o), SHADOW);
        }
    }

    EndWindow95();
}

void DrawExplorers(AppState& app) {
    for (auto& w : app.wins)
        if (w.open && !w.minimized) DrawExplorer(app, w);
    for (size_t i = 0; i < app.wins.size();) {
        if (!app.wins[i].open) app.wins.erase(app.wins.begin() + (long)i);
        else ++i;
    }
}
