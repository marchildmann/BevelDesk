#include "shell/macdemo.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include <cmath>
#include <ctime>

// A classic Macintosh System 1 (1984) desktop. Pure black & white, square
// corners, 1px lines — the aesthetic opposite of BevelDesk's 3D chrome, which
// is exactly why it's the right test of the theme seam. Self-contained spike:
// draws straight to the window draw list, bypassing the Win95 shell.

using t95::AddTextBold;

static const ImU32 BK = IM_COL32(0, 0, 0, 255);
static const ImU32 WH = IM_COL32(255, 255, 255, 255);

static inline ImVec2 A(ImVec2 p, float x, float y) { return ImVec2(p.x + x, p.y + y); }

// ---- procedural 1-bit icons (line art, like the originals) ------------------

static void MacFolder(ImDrawList* dl, ImVec2 p) {           // ~32x24
    dl->AddRectFilled(A(p, 0, 4), A(p, 32, 24), WH);
    dl->AddRect(A(p, 0, 4), A(p, 32, 24), BK);
    dl->AddLine(A(p, 2, 4), A(p, 12, 4), BK);               // tab
    dl->AddLine(A(p, 2, 1), A(p, 12, 1), BK);
    dl->AddLine(A(p, 2, 1), A(p, 2, 4), BK);
    dl->AddLine(A(p, 12, 1), A(p, 14, 4), BK);
    dl->AddLine(A(p, 0, 8), A(p, 32, 8), BK);               // lip
}

static void MacDoc(ImDrawList* dl, ImVec2 p) {              // ~24x30
    dl->AddRectFilled(A(p, 0, 0), A(p, 18, 30), WH);
    dl->AddLine(A(p, 0, 0), A(p, 13, 0), BK);
    dl->AddLine(A(p, 0, 0), A(p, 0, 30), BK);
    dl->AddLine(A(p, 0, 30), A(p, 18, 30), BK);
    dl->AddLine(A(p, 18, 30), A(p, 18, 6), BK);
    dl->AddLine(A(p, 13, 0), A(p, 18, 6), BK);              // folded corner
    dl->AddLine(A(p, 13, 0), A(p, 13, 6), BK);
    dl->AddLine(A(p, 13, 6), A(p, 18, 6), BK);
    for (int i = 0; i < 4; ++i)                             // text lines
        dl->AddLine(A(p, 3, 11 + i * 4.0f), A(p, 15, 11 + i * 4.0f), BK);
}

static void MacDisk(ImDrawList* dl, ImVec2 p) {             // hard disk ~40x28
    dl->AddRectFilled(A(p, 0, 0), A(p, 40, 24), WH);
    dl->AddRect(A(p, 0, 0), A(p, 40, 24), BK);
    dl->AddRect(A(p, 0, 0), A(p, 40, 24), BK, 0, 0, 1.0f);
    dl->AddLine(A(p, 0, 17), A(p, 40, 17), BK);
    dl->AddRectFilled(A(p, 5, 20), A(p, 12, 22), BK);       // slot
    dl->AddRect(A(p, 30, 3), A(p, 36, 14), BK);             // label
}

static void MacTrash(ImDrawList* dl, ImVec2 p) {           // ~26x30
    dl->AddLine(A(p, 3, 6), A(p, 23, 6), BK);              // lid
    dl->AddLine(A(p, 5, 3), A(p, 21, 3), BK);
    dl->AddLine(A(p, 5, 3), A(p, 5, 6), BK);
    dl->AddLine(A(p, 21, 3), A(p, 21, 6), BK);
    dl->AddLine(A(p, 4, 6), A(p, 6, 30), BK);              // body sides
    dl->AddLine(A(p, 22, 6), A(p, 20, 30), BK);
    dl->AddLine(A(p, 6, 30), A(p, 20, 30), BK);            // bottom
    for (int i = 0; i < 3; ++i)                            // ribs
        dl->AddLine(A(p, 8 + i * 5.0f, 9), A(p, 8 + i * 5.0f, 27), BK);
}

// small Apple logo for the menu bar (~14x16), solid black silhouette
static void AppleLogo(ImDrawList* dl, ImVec2 p) {
    dl->AddCircleFilled(A(p, 5, 9), 4.5f, BK, 16);
    dl->AddCircleFilled(A(p, 10, 9), 4.5f, BK, 16);
    dl->AddRectFilled(A(p, 3, 5), A(p, 12, 14), BK);
    dl->AddRectFilled(A(p, 6.5f, 8), A(p, 8.5f, 16), WH);  // bite/notch shaping
    dl->AddRectFilled(A(p, 6, 3), A(p, 8, 6), WH);
    dl->AddLine(A(p, 8, 0), A(p, 10, 4), BK);              // leaf/stem
    dl->AddLine(A(p, 9, 1), A(p, 11, 4), BK);
}

// ---- chrome -----------------------------------------------------------------

// Classic Mac window title bar: six horizontal "racing stripes", a close box at
// the left, and a centered title in a white box that interrupts the stripes.
static void MacTitleBar(ImDrawList* dl, ImVec2 wmin, float w, const char* title,
                        ImFont* font) {
    float th = 19;
    ImVec2 tmax = A(wmin, w, th);
    dl->AddRectFilled(wmin, tmax, WH);
    for (int i = 0; i < 6; ++i) {                          // stripes
        float yy = wmin.y + 3 + i * 2.0f;
        dl->AddLine(ImVec2(wmin.x + 1, yy), ImVec2(tmax.x - 1, yy), BK);
    }
    dl->AddRect(wmin, tmax, BK);                           // frame + bottom rule
    // close box (top-left)
    ImVec2 cb0 = A(wmin, 8, 5), cb1 = A(wmin, 19, 16);
    dl->AddRectFilled(cb0, cb1, WH);
    dl->AddRect(cb0, cb1, BK);
    // centered title box over the stripes
    ImVec2 ts = font->CalcTextSizeA(font->LegacySize, FLT_MAX, 0.0f, title);
    float bw = ts.x + 16;
    ImVec2 pillmin(wmin.x + (w - bw) * 0.5f, wmin.y + 2);
    ImVec2 pillmax(pillmin.x + bw, wmin.y + th - 2);
    dl->AddRectFilled(pillmin, pillmax, WH);
    AddTextBold(dl, ImVec2(pillmin.x + 8, pillmin.y + (th - 4 - ts.y) * 0.5f), BK, title);
}

// A System-1 window: hard 1px drop shadow, white body, square corners.
static void MacWindow(ImDrawList* dl, ImVec2 mn, ImVec2 mx, const char* title,
                      ImFont* font) {
    // hard offset shadow (solid, no blur)
    dl->AddRectFilled(ImVec2(mn.x + 2, mn.y + 2), ImVec2(mx.x + 2, mx.y + 2), BK);
    dl->AddRectFilled(mn, mx, WH);
    dl->AddRect(mn, mx, BK);
    MacTitleBar(dl, mn, mx.x - mn.x, title, font);
    // resize grip (bottom-right)
    ImVec2 g0(mx.x - 15, mx.y - 15);
    dl->AddRectFilled(g0, mx, WH);
    dl->AddRect(g0, mx, BK);
    dl->AddRect(ImVec2(g0.x + 3, g0.y + 3), ImVec2(mx.x - 3, mx.y - 3), BK);
}

// ---- the desktop ------------------------------------------------------------

void DrawMacDesktop(AppState& app) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 1, 1));
    ImGui::Begin("##macdesk", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImFont* font = t95::FontBold ? t95::FontBold : ImGui::GetFont();
    ImVec2 o = vp->Pos, sz = vp->Size;

    // desktop: the classic 50% gray, drawn as a 2px checkerboard of 1px dots
    // (two offset grids). Direct draw — reliable, unlike GL_REPEAT tiling under
    // ImGui's texture manager. Batched into the window draw list.
    dl->AddRectFilled(o, ImVec2(o.x + sz.x, o.y + sz.y), WH);
    {
        float x0 = o.x, x1 = o.x + sz.x, y0 = o.y + 20, y1 = o.y + sz.y;
        for (float yy = y0; yy < y1; yy += 2) {
            float xoff = (((int)((yy - y0)) / 2) & 1) ? 1.0f : 0.0f;
            for (float xx = x0 + xoff; xx < x1; xx += 2)
                dl->AddRectFilled(ImVec2(xx, yy), ImVec2(xx + 1, yy + 1), BK);
        }
    }

    // menu bar (white, 20px, black bottom rule)
    dl->AddRectFilled(o, ImVec2(o.x + sz.x, o.y + 20), WH);
    dl->AddLine(ImVec2(o.x, o.y + 20), ImVec2(o.x + sz.x, o.y + 20), BK);
    AppleLogo(dl, ImVec2(o.x + 10, o.y + 2));
    const char* menus[] = { "File", "Edit", "View", "Special" };
    float mx = o.x + 34;
    for (const char* m : menus) {
        AddTextBold(dl, ImVec2(mx, o.y + 3), BK, m);
        mx += ImGui::CalcTextSize(m).x + 20;
    }
    // clock at the far right (System 6+ flourish, but a nice touch)
    {
        time_t t = time(nullptr); struct tm* lt = localtime(&t);
        char buf[16]; strftime(buf, sizeof(buf), "%I:%M %p", lt);
        const char* c = (buf[0] == '0') ? buf + 1 : buf;
        AddTextBold(dl, ImVec2(o.x + sz.x - ImGui::CalcTextSize(c).x - 14, o.y + 3), BK, c);
    }

    // disk + trash icons on the desktop
    MacDisk(dl, ImVec2(o.x + sz.x - 66, o.y + 34));
    AddTextBold(dl, ImVec2(o.x + sz.x - 78, o.y + 62), BK, "Macintosh HD");
    MacTrash(dl, ImVec2(o.x + sz.x - 52, o.y + sz.y - 74));
    AddTextBold(dl, ImVec2(o.x + sz.x - 54, o.y + sz.y - 42), BK, "Trash");

    // an open Finder window
    ImVec2 wmn(o.x + 60, o.y + 54), wmx(o.x + 430, o.y + 320);
    MacWindow(dl, wmn, wmx, "Macintosh HD", font);
    // "N items" header line
    dl->AddLine(ImVec2(wmn.x + 1, wmn.y + 37), ImVec2(wmx.x - 1, wmn.y + 37), BK);
    AddTextBold(dl, ImVec2(wmn.x + 8, wmn.y + 24), BK, "3 items    398K in disk");

    // icon grid inside the window
    struct It { const char* name; int kind; };  // 0 folder,1 doc
    It items[] = { {"System Folder", 0}, {"Applications", 0}, {"Read Me", 1} };
    float ix = wmn.x + 30, iy = wmn.y + 60;
    for (const It& it : items) {
        if (it.kind == 0) MacFolder(dl, ImVec2(ix, iy + 4));
        else              MacDoc(dl, ImVec2(ix + 4, iy));
        ImVec2 ns = ImGui::CalcTextSize(it.name);
        float tx = ix + 16 - ns.x * 0.5f;
        // selected-look name tag on the first item (inverted)
        if (it.kind == 0 && it.name[0] == 'S') {
            dl->AddRectFilled(ImVec2(tx - 3, iy + 36), ImVec2(tx + ns.x + 3, iy + 50), BK);
            AddTextBold(dl, ImVec2(tx, iy + 37), WH, it.name);
        } else {
            AddTextBold(dl, ImVec2(tx, iy + 37), BK, it.name);
        }
        ix += 100;
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}
