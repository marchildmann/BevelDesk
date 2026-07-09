#include "theme95/icons95.h"
#include "theme95/palette.h"

namespace icons95 {
using namespace t95;

static inline ImVec2 A(ImVec2 p, float x, float y) { return ImVec2(p.x + x, p.y + y); }

void MyComputer32(ImDrawList* dl, ImVec2 p) {
    // monitor
    dl->AddRectFilled(A(p, 3, 0), A(p, 29, 19), FACE);
    dl->AddRect(A(p, 3, 0), A(p, 29, 19), BLACK);
    dl->AddLine(A(p, 4, 1), A(p, 27, 1), HILIGHT);
    dl->AddLine(A(p, 4, 1), A(p, 4, 17), HILIGHT);
    // screen (navy, with a tiny teal "desktop" block)
    dl->AddRectFilled(A(p, 6, 3), A(p, 26, 16), NAVY);
    dl->AddRect(A(p, 6, 3), A(p, 26, 16), SHADOW);
    dl->AddRectFilled(A(p, 8, 5), A(p, 16, 10), DESKTOP);
    dl->AddRectFilled(A(p, 9, 6), A(p, 15, 7), HILIGHT);
    // stand
    dl->AddRectFilled(A(p, 13, 19), A(p, 19, 21), SHADOW);
    // desktop case
    dl->AddRectFilled(A(p, 1, 21), A(p, 31, 30), FACE);
    dl->AddRect(A(p, 1, 21), A(p, 31, 30), BLACK);
    dl->AddLine(A(p, 2, 22), A(p, 29, 22), HILIGHT);
    dl->AddLine(A(p, 2, 22), A(p, 2, 28), HILIGHT);
    dl->AddLine(A(p, 2, 29), A(p, 30, 29), SHADOW);
    // floppy slot + LED + button
    dl->AddRectFilled(A(p, 5, 24), A(p, 19, 26), SHADOW);
    dl->AddRectFilled(A(p, 23, 24), A(p, 26, 26), GREEN);
    dl->AddRectFilled(A(p, 27, 24), A(p, 29, 26), SHADOW);
}

void RecycleBin32(ImDrawList* dl, ImVec2 p) {
    // crumpled paper poking out
    dl->AddRectFilled(A(p, 13, 1), A(p, 20, 6), HILIGHT);
    dl->AddRect(A(p, 13, 1), A(p, 20, 6), SHADOW);
    // basket body (tapered)
    ImVec2 body[4] = { A(p, 7, 8), A(p, 25, 8), A(p, 22, 30), A(p, 10, 30) };
    dl->AddConvexPolyFilled(body, 4, FACE);
    dl->AddPolyline(body, 4, BLACK, ImDrawFlags_Closed, 1.0f);
    // vertical shading stripes
    dl->AddLine(A(p, 11, 10), A(p, 12, 28), SHADOW);
    dl->AddLine(A(p, 15, 10), A(p, 15, 28), SHADOW);
    dl->AddLine(A(p, 19, 10), A(p, 19, 28), SHADOW);
    dl->AddLine(A(p, 22, 10), A(p, 21, 28), SHADOW);
    dl->AddLine(A(p, 13, 10), A(p, 13, 28), HILIGHT);
    dl->AddLine(A(p, 17, 10), A(p, 17, 28), HILIGHT);
    // elliptical opening with a lighter rim
    dl->AddEllipseFilled(A(p, 16, 8), ImVec2(9, 3), SHADOW);
    dl->AddEllipse(A(p, 16, 8), ImVec2(9, 3), BLACK);
    dl->AddEllipse(A(p, 16, 7), ImVec2(9, 3), HILIGHT);
    dl->AddEllipse(A(p, 16, 8), ImVec2(6, 1.5f), DKSHADOW);
    // base rim
    dl->AddLine(A(p, 10, 29), A(p, 22, 29), SHADOW);
}

void Folder32(ImDrawList* dl, ImVec2 p) {
    // back tab
    dl->AddRectFilled(A(p, 1, 6), A(p, 14, 11), FOLDER);
    dl->AddRect(A(p, 1, 6), A(p, 14, 11), FOLDER_EDGE);
    // body
    dl->AddRectFilled(A(p, 1, 9), A(p, 31, 27), FOLDER);
    dl->AddRect(A(p, 1, 9), A(p, 31, 27), FOLDER_EDGE);
    dl->AddLine(A(p, 2, 10), A(p, 29, 10), HILIGHT);
    dl->AddLine(A(p, 2, 10), A(p, 2, 25), HILIGHT);
    dl->AddLine(A(p, 2, 26), A(p, 30, 26), FOLDER_SHADE);
}

void Document32(ImDrawList* dl, ImVec2 p) {
    // page with folded corner
    dl->AddRectFilled(A(p, 6, 1), A(p, 26, 31), HILIGHT);
    dl->AddRect(A(p, 6, 1), A(p, 26, 31), BLACK);
    dl->AddTriangleFilled(A(p, 19, 1), A(p, 26, 8), A(p, 19, 8), FACE);
    dl->AddLine(A(p, 19, 1), A(p, 19, 8), BLACK);
    dl->AddLine(A(p, 19, 8), A(p, 26, 8), BLACK);
    for (int i = 0; i < 5; ++i)
        dl->AddLine(A(p, 9, 12 + i * 3.0f), A(p, 22, 12 + i * 3.0f), SHADOW);
    dl->AddLine(A(p, 9, 27), A(p, 17, 27), SHADOW);
}

void Folder14(ImDrawList* dl, ImVec2 p) {
    // back tab
    dl->AddRectFilled(A(p, 0, 2), A(p, 6, 5), FOLDER);
    dl->AddRect(A(p, 0, 2), A(p, 6, 5), FOLDER_EDGE);
    // body
    dl->AddRectFilled(A(p, 0, 4), A(p, 13, 12), FOLDER);
    dl->AddRect(A(p, 0, 4), A(p, 13, 12), FOLDER_EDGE);
    dl->AddLine(A(p, 1, 5), A(p, 12, 5), HILIGHT);
}

void Document14(ImDrawList* dl, ImVec2 p) {
    // page with folded corner
    dl->AddRectFilled(A(p, 2, 0), A(p, 11, 13), HILIGHT);
    dl->AddRect(A(p, 2, 0), A(p, 11, 13), BLACK);
    dl->AddTriangleFilled(A(p, 8, 0), A(p, 11, 3), A(p, 8, 3), FACE);
    dl->AddLine(A(p, 8, 0), A(p, 8, 3), BLACK);
    dl->AddLine(A(p, 8, 3), A(p, 11, 3), BLACK);
    // text lines
    dl->AddLine(A(p, 4, 5), A(p, 9, 5), SHADOW);
    dl->AddLine(A(p, 4, 7), A(p, 9, 7), SHADOW);
    dl->AddLine(A(p, 4, 9), A(p, 8, 9), SHADOW);
}

void MiniComputer14(ImDrawList* dl, ImVec2 p) {
    dl->AddRectFilled(A(p, 1, 1), A(p, 13, 9), FACE);
    dl->AddRect(A(p, 1, 1), A(p, 13, 9), BLACK);
    dl->AddRectFilled(A(p, 3, 3), A(p, 11, 7), DESKTOP);
    dl->AddRectFilled(A(p, 5, 9), A(p, 9, 10), SHADOW);
    dl->AddRectFilled(A(p, 3, 10), A(p, 11, 12), FACE);
    dl->AddRect(A(p, 3, 10), A(p, 11, 12), BLACK);
}

void UpFolderGlyph(ImDrawList* dl, ImVec2 p) {
    // small folder, bottom-left of the 24x22 button
    Folder14(dl, A(p, 3, 6));
    // up arrow above/right of it
    dl->AddRectFilled(A(p, 15, 8), A(p, 17, 15), BLACK);
    dl->AddTriangleFilled(A(p, 12, 8), A(p, 20, 8), A(p, 16, 3), BLACK);
}

void LargeIconsGlyph(ImDrawList* dl, ImVec2 p) {
    // 2x2 grid of little "icons"
    const ImU32 B = ACCENT;
    dl->AddRectFilled(A(p, 5, 4), A(p, 10, 9), B);
    dl->AddRectFilled(A(p, 14, 4), A(p, 19, 9), B);
    dl->AddRectFilled(A(p, 5, 13), A(p, 10, 18), B);
    dl->AddRectFilled(A(p, 14, 13), A(p, 19, 18), B);
}

void MiniDos14(ImDrawList* dl, ImVec2 p) {
    // MS-DOS window: gray frame, black screen, gray prompt lines + cursor
    dl->AddRectFilled(A(p, 0, 1), A(p, 14, 12), FACE);
    dl->AddRect(A(p, 0, 1), A(p, 14, 12), BLACK);
    dl->AddRectFilled(A(p, 2, 3), A(p, 12, 10), BLACK);
    dl->AddRectFilled(A(p, 3, 4), A(p, 6, 5), IM_COL32(170, 170, 170, 255));
    dl->AddRectFilled(A(p, 3, 6), A(p, 8, 7), IM_COL32(170, 170, 170, 255));
    dl->AddRectFilled(A(p, 3, 8), A(p, 5, 9), HILIGHT);
}

void DetailsGlyph(ImDrawList* dl, ImVec2 p) {
    // three rows: bullet + line
    const ImU32 B = ACCENT;
    for (int i = 0; i < 3; ++i) {
        float y = 5 + i * 5.0f;
        dl->AddRectFilled(A(p, 5, y), A(p, 8, y + 3), B);
        dl->AddLine(A(p, 10, y + 1), A(p, 19, y + 1), BLACK);
    }
}

void BevelDeskLogo(ImDrawList* dl, ImVec2 p) {
    // four raised beveled tiles — the mark IS the bevel; teal = the desk
    auto tile = [&](float x, float y, ImU32 fill) {
        dl->AddRectFilled(A(p, x, y), A(p, x + 6, y + 6), fill);
        dl->AddRectFilled(A(p, x, y), A(p, x + 5, y + 1), HILIGHT);      // top
        dl->AddRectFilled(A(p, x, y), A(p, x + 1, y + 5), HILIGHT);      // left
        dl->AddRectFilled(A(p, x, y + 5), A(p, x + 6, y + 6), DKSHADOW); // bottom
        dl->AddRectFilled(A(p, x + 5, y), A(p, x + 6, y + 6), DKSHADOW); // right
    };
    tile(0, 0, DESKTOP);
    tile(7, 0, FACE);
    tile(0, 7, FACE);
    tile(7, 7, FACE);
}

} // namespace icons95
