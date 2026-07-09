#include "theme95/bevel.h"

namespace t95 {

void Edge(ImDrawList* dl, ImVec2 mn, ImVec2 mx, ImU32 tl, ImU32 br) {
    // top + left in tl color, bottom + right in br color (BR wins corners
    // on bottom/right, matching DrawEdge output closely enough).
    dl->AddRectFilled(ImVec2(mn.x, mn.y), ImVec2(mx.x - 1, mn.y + 1), tl); // top
    dl->AddRectFilled(ImVec2(mn.x, mn.y), ImVec2(mn.x + 1, mx.y - 1), tl); // left
    dl->AddRectFilled(ImVec2(mn.x, mx.y - 1), ImVec2(mx.x, mx.y), br);     // bottom
    dl->AddRectFilled(ImVec2(mx.x - 1, mn.y), ImVec2(mx.x, mx.y), br);     // right
}

// Inset helpers keep the chiseled branches readable.
static inline ImVec2 In(ImVec2 p, float d) { return ImVec2(p.x + d, p.y + d); }
static inline ImVec2 Out(ImVec2 p, float d) { return ImVec2(p.x - d, p.y - d); }

void BevelRaised(ImDrawList* dl, ImVec2 mn, ImVec2 mx, bool fill) {
    if (fill) dl->AddRectFilled(mn, mx, FACE);
    if (Sty.chiseled) {                                   // NeXT: black keyline + 1px bevel
        Edge(dl, mn, mx, DKSHADOW, DKSHADOW);
        Edge(dl, In(mn, 1), Out(mx, 1), HILIGHT, SHADOW);
    } else {                                              // Win95: two-edge 3D bevel
        Edge(dl, mn, mx, HILIGHT, DKSHADOW);
        Edge(dl, In(mn, 1), Out(mx, 1), LIGHT, SHADOW);
    }
}

void BevelPressed(ImDrawList* dl, ImVec2 mn, ImVec2 mx, bool fill) {
    if (fill) dl->AddRectFilled(mn, mx, FACE);
    if (Sty.chiseled) {
        Edge(dl, mn, mx, DKSHADOW, DKSHADOW);
        Edge(dl, In(mn, 1), Out(mx, 1), SHADOW, HILIGHT);
    } else {
        Edge(dl, mn, mx, DKSHADOW, HILIGHT);
        Edge(dl, In(mn, 1), Out(mx, 1), SHADOW, LIGHT);
    }
}

void SunkenField(ImDrawList* dl, ImVec2 mn, ImVec2 mx, ImU32 fill_col) {
    dl->AddRectFilled(mn, mx, fill_col);
    if (Sty.chiseled) {
        Edge(dl, mn, mx, DKSHADOW, DKSHADOW);
        Edge(dl, In(mn, 1), Out(mx, 1), SHADOW, LIGHT);
    } else {
        Edge(dl, mn, mx, SHADOW, HILIGHT);
        Edge(dl, In(mn, 1), Out(mx, 1), DKSHADOW, LIGHT);
    }
}

void WindowFrame(ImDrawList* dl, ImVec2 mn, ImVec2 mx, bool fill) {
    if (fill) dl->AddRectFilled(mn, mx, FACE);
    if (Sty.chiseled) {
        Edge(dl, mn, mx, DKSHADOW, DKSHADOW);
        Edge(dl, In(mn, 1), Out(mx, 1), HILIGHT, SHADOW);
    } else {
        Edge(dl, mn, mx, LIGHT, DKSHADOW);
        Edge(dl, In(mn, 1), Out(mx, 1), HILIGHT, SHADOW);
    }
}

void ThinSunken(ImDrawList* dl, ImVec2 mn, ImVec2 mx) {
    Edge(dl, mn, mx, SHADOW, HILIGHT);
}

void ThinRaised(ImDrawList* dl, ImVec2 mn, ImVec2 mx) {
    Edge(dl, mn, mx, HILIGHT, SHADOW);
}

void DitherFill(ImDrawList* dl, ImVec2 mn, ImVec2 mx) {
    dl->AddRectFilled(mn, mx, FACE);
    for (int y = (int)mn.y; y < (int)mx.y; ++y)
        for (int x = (int)mn.x + ((y + (int)mn.x) & 1); x < (int)mx.x; x += 2)
            dl->AddRectFilled(ImVec2((float)x, (float)y), ImVec2((float)x + 1, (float)y + 1), HILIGHT);
}

} // namespace t95
