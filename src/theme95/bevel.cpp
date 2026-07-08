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

void BevelRaised(ImDrawList* dl, ImVec2 mn, ImVec2 mx, bool fill) {
    if (fill) dl->AddRectFilled(mn, mx, FACE);
    Edge(dl, mn, mx, HILIGHT, DKSHADOW);
    Edge(dl, ImVec2(mn.x + 1, mn.y + 1), ImVec2(mx.x - 1, mx.y - 1), LIGHT, SHADOW);
}

void BevelPressed(ImDrawList* dl, ImVec2 mn, ImVec2 mx, bool fill) {
    if (fill) dl->AddRectFilled(mn, mx, FACE);
    Edge(dl, mn, mx, DKSHADOW, HILIGHT);
    Edge(dl, ImVec2(mn.x + 1, mn.y + 1), ImVec2(mx.x - 1, mx.y - 1), SHADOW, LIGHT);
}

void SunkenField(ImDrawList* dl, ImVec2 mn, ImVec2 mx, ImU32 fill_col) {
    dl->AddRectFilled(mn, mx, fill_col);
    Edge(dl, mn, mx, SHADOW, HILIGHT);
    Edge(dl, ImVec2(mn.x + 1, mn.y + 1), ImVec2(mx.x - 1, mx.y - 1), DKSHADOW, LIGHT);
}

void WindowFrame(ImDrawList* dl, ImVec2 mn, ImVec2 mx, bool fill) {
    if (fill) dl->AddRectFilled(mn, mx, FACE);
    Edge(dl, mn, mx, LIGHT, DKSHADOW);
    Edge(dl, ImVec2(mn.x + 1, mn.y + 1), ImVec2(mx.x - 1, mx.y - 1), HILIGHT, SHADOW);
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
