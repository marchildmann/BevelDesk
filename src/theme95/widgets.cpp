#include "theme95/widgets.h"
#include "theme95/bevel.h"

namespace t95 {

ImFont* FontBold = nullptr;
ImFont* FontMono = nullptr;

void AddTextBold(ImDrawList* dl, ImVec2 pos, ImU32 col, const char* text,
                 const ImVec4* clip) {
    if (FontBold) {
        dl->AddText(FontBold, 0.0f, pos, col, text, nullptr, 0.0f, clip);
    } else {
        dl->AddText(nullptr, 0.0f, pos, col, text, nullptr, 0.0f, clip);
        dl->AddText(nullptr, 0.0f, ImVec2(pos.x + 1, pos.y), col, text, nullptr, 0.0f, clip);
    }
}

bool Button(const char* label, ImVec2 size, bool forced_pressed, bool enabled) {
    ImVec2 ts = ImGui::CalcTextSize(label, nullptr, true);
    if (size.x <= 0) size.x = (ts.x + 24 < 75) ? 75 : ts.x + 24;
    if (size.y <= 0) size.y = 23;
    ImVec2 p = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(label, size);
    if (!enabled) clicked = false;
    bool pressed = enabled && (forced_pressed || (ImGui::IsItemActive() && ImGui::IsItemHovered()));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mx(p.x + size.x, p.y + size.y);
    if (pressed) BevelPressed(dl, p, mx); else BevelRaised(dl, p, mx);
    float off = pressed ? 1.0f : 0.0f;
    ImVec2 tp(p.x + (size.x - ts.x) * 0.5f + off, p.y + (size.y - ts.y) * 0.5f + off);
    if (enabled) {
        dl->AddText(tp, TEXT, label);
    } else {
        // Win95 disabled label: gray with a white emboss offset +1,+1
        dl->AddText(ImVec2(tp.x + 1, tp.y + 1), HILIGHT, label);
        dl->AddText(tp, GRAYTEXT, label);
    }
    return clicked;
}

bool Radio(const char* label, bool selected) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 ts = ImGui::CalcTextSize(label);
    float h = 14;
    bool clicked = ImGui::InvisibleButton(label, ImVec2(18 + ts.x, h));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 c(p.x + 6, p.y + 7);
    dl->AddCircleFilled(c, 5.5f, WINBG, 12);
    // sunken ring: dark top-left arc, light bottom-right arc
    dl->PathArcTo(c, 5.5f, 3.1415926f * 0.75f, 3.1415926f * 1.75f, 8);
    dl->PathStroke(SHADOW, 0, 1.0f);
    dl->PathArcTo(c, 5.5f, -3.1415926f * 0.25f, 3.1415926f * 0.75f, 8);
    dl->PathStroke(HILIGHT, 0, 1.0f);
    dl->PathArcTo(c, 4.5f, 3.1415926f * 0.75f, 3.1415926f * 1.75f, 8);
    dl->PathStroke(DKSHADOW, 0, 1.0f);
    if (selected) dl->AddCircleFilled(c, 2.5f, BLACK, 8);
    dl->AddText(ImVec2(p.x + 18, p.y + (h - ImGui::GetFontSize()) * 0.5f), TEXT, label);
    return clicked;
}

bool ToolButton(const char* id, ImVec2 size, void (*glyph)(ImDrawList*, ImVec2)) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, size);
    bool hovered = ImGui::IsItemHovered();
    bool held = ImGui::IsItemActive() && hovered;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mx(p.x + size.x, p.y + size.y);
    dl->AddRectFilled(p, mx, FACE);
    if (held)         ThinSunken(dl, p, mx);
    else if (hovered) ThinRaised(dl, p, mx);
    float off = held ? 1.0f : 0.0f;
    if (glyph) glyph(dl, ImVec2(p.x + off, p.y + off));
    return clicked;
}

static void CaptionGlyph(ImDrawList* dl, ImVec2 p, Glyph g) {
    switch (g) {
    case GLYPH_CLOSE:
        // 2px-thick X
        dl->AddLine(ImVec2(p.x + 4, p.y + 3), ImVec2(p.x + 10, p.y + 9), BLACK);
        dl->AddLine(ImVec2(p.x + 5, p.y + 3), ImVec2(p.x + 11, p.y + 9), BLACK);
        dl->AddLine(ImVec2(p.x + 10, p.y + 3), ImVec2(p.x + 4, p.y + 9), BLACK);
        dl->AddLine(ImVec2(p.x + 11, p.y + 3), ImVec2(p.x + 5, p.y + 9), BLACK);
        break;
    case GLYPH_MIN:
        dl->AddRectFilled(ImVec2(p.x + 4, p.y + 9), ImVec2(p.x + 10, p.y + 11), BLACK);
        break;
    case GLYPH_MAX:
        dl->AddRect(ImVec2(p.x + 3, p.y + 2), ImVec2(p.x + 12, p.y + 11), BLACK);
        dl->AddRectFilled(ImVec2(p.x + 3, p.y + 2), ImVec2(p.x + 12, p.y + 4), BLACK);
        break;
    case GLYPH_RESTORE:
        dl->AddRect(ImVec2(p.x + 5, p.y + 2), ImVec2(p.x + 12, p.y + 8), BLACK);
        dl->AddRectFilled(ImVec2(p.x + 5, p.y + 2), ImVec2(p.x + 12, p.y + 4), BLACK);
        dl->AddRectFilled(ImVec2(p.x + 3, p.y + 5), ImVec2(p.x + 10, p.y + 7), FACE);
        dl->AddRect(ImVec2(p.x + 3, p.y + 5), ImVec2(p.x + 10, p.y + 11), BLACK);
        dl->AddRectFilled(ImVec2(p.x + 3, p.y + 5), ImVec2(p.x + 10, p.y + 7), BLACK);
        break;
    }
}

bool CaptionButton(const char* id, ImVec2 pos, Glyph glyph) {
    ImGui::SetCursorScreenPos(pos);
    bool clicked = ImGui::InvisibleButton(id, ImVec2(CAPBTN_W, CAPBTN_H));
    bool held = ImGui::IsItemActive() && ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mx(pos.x + CAPBTN_W, pos.y + CAPBTN_H);
    if (held) BevelPressed(dl, pos, mx); else BevelRaised(dl, pos, mx);
    float off = held ? 1.0f : 0.0f;
    CaptionGlyph(dl, ImVec2(pos.x + off, pos.y + off), glyph);
    return clicked;
}

static void ArrowGlyph(ImDrawList* dl, ImVec2 center, bool up) {
    float d = up ? 1.0f : -1.0f;
    dl->AddTriangleFilled(ImVec2(center.x - 4, center.y + 2 * d),
                          ImVec2(center.x + 4, center.y + 2 * d),
                          ImVec2(center.x, center.y - 2 * d), BLACK);
}

float ScrollBarV(const char* id, ImVec2 mn, ImVec2 mx,
                 float view_h, float content_h, float scroll) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float W = mx.x - mn.x;
    const float BTN = 16.0f;
    float max_scroll = content_h - view_h;
    if (max_scroll < 0) max_scroll = 0;

    ImGui::PushID(id);

    // up / down arrow buttons
    ImVec2 up_mn = mn, up_mx = ImVec2(mx.x, mn.y + BTN);
    ImVec2 dn_mn = ImVec2(mn.x, mx.y - BTN), dn_mx = mx;
    ImGui::SetCursorScreenPos(up_mn);
    ImGui::InvisibleButton("up", ImVec2(W, BTN));
    bool up_held = ImGui::IsItemActive() && ImGui::IsItemHovered();
    if (up_held) scroll -= 17.0f * ImGui::GetIO().DeltaTime * 12.0f;
    ImGui::SetCursorScreenPos(dn_mn);
    ImGui::InvisibleButton("dn", ImVec2(W, BTN));
    bool dn_held = ImGui::IsItemActive() && ImGui::IsItemHovered();
    if (dn_held) scroll += 17.0f * ImGui::GetIO().DeltaTime * 12.0f;

    // track geometry
    float track_top = up_mx.y, track_bot = dn_mn.y;
    float track_h = track_bot - track_top;
    float thumb_h = (content_h > 0) ? track_h * (view_h / content_h) : track_h;
    if (thumb_h < 17) thumb_h = 17;
    if (thumb_h > track_h) thumb_h = track_h;
    float t = (max_scroll > 0) ? (scroll / max_scroll) : 0.0f;
    if (t < 0) t = 0; if (t > 1) t = 1;
    float thumb_top = track_top + t * (track_h - thumb_h);

    // thumb drag — submitted BEFORE the track: the first item to claim the
    // click gets the active id, and ImGui then blocks hover on the item
    // underneath. Track-first meant the thumb could never be grabbed.
    ImGui::SetCursorScreenPos(ImVec2(mn.x, thumb_top));
    ImGui::InvisibleButton("thumb", ImVec2(W, thumb_h));
    if (ImGui::IsItemActive() && track_h > thumb_h)
        scroll += ImGui::GetIO().MouseDelta.y * (max_scroll / (track_h - thumb_h));

    // track paging (only receives clicks outside the thumb)
    ImGui::SetCursorScreenPos(ImVec2(mn.x, track_top));
    ImGui::InvisibleButton("track", ImVec2(W, track_h > 0 ? track_h : 1));
    if (ImGui::IsItemClicked()) {
        float my = ImGui::GetIO().MousePos.y;
        if (my < thumb_top) scroll -= view_h;
        else if (my >= thumb_top + thumb_h) scroll += view_h;
    }

    if (scroll < 0) scroll = 0;
    if (scroll > max_scroll) scroll = max_scroll;

    // ---- draw ----
    DitherFill(dl, ImVec2(mn.x, track_top), ImVec2(mx.x, track_bot));
    if (up_held) BevelPressed(dl, up_mn, up_mx); else BevelRaised(dl, up_mn, up_mx);
    if (dn_held) BevelPressed(dl, dn_mn, dn_mx); else BevelRaised(dl, dn_mn, dn_mx);
    ArrowGlyph(dl, ImVec2((up_mn.x + up_mx.x) * 0.5f + (up_held ? 1 : 0),
                          (up_mn.y + up_mx.y) * 0.5f + (up_held ? 1 : 0)), true);
    ArrowGlyph(dl, ImVec2((dn_mn.x + dn_mx.x) * 0.5f + (dn_held ? 1 : 0),
                          (dn_mn.y + dn_mx.y) * 0.5f + (dn_held ? 1 : 0)), false);
    // recompute thumb pos after clamping so it doesn't lag
    t = (max_scroll > 0) ? (scroll / max_scroll) : 0.0f;
    thumb_top = track_top + t * (track_h - thumb_h);
    if (track_h > 0)
        BevelRaised(dl, ImVec2(mn.x, thumb_top), ImVec2(mx.x, thumb_top + thumb_h));

    ImGui::PopID();
    return scroll;
}

} // namespace t95
