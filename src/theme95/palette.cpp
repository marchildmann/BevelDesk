#include "theme95/palette.h"
#include "theme95/style.h"

namespace t95 {

// Live chrome colors — initialized to Silver so the default look is byte-identical
// to before this file existed. ApplyScheme() reassigns them.
ImU32 FACE     = IM_COL32(192, 192, 192, 255);
ImU32 LIGHT    = IM_COL32(223, 223, 223, 255);
ImU32 HILIGHT  = IM_COL32(255, 255, 255, 255);
ImU32 SHADOW   = IM_COL32(128, 128, 128, 255);
ImU32 DKSHADOW = IM_COL32(0, 0, 0, 255);
ImU32 TITLE_ACT    = IM_COL32(0, 0, 128, 255);
ImU32 TITLE_ACT2   = IM_COL32(16, 132, 208, 255);
ImU32 TITLE_INACT  = IM_COL32(128, 128, 128, 255);
ImU32 TITLE_INACT2 = IM_COL32(181, 181, 181, 255);
ImU32 TITLE_TEXT   = IM_COL32(255, 255, 255, 255);
ImU32 DESKTOP  = IM_COL32(0, 128, 128, 255);
ImU32 WINBG    = IM_COL32(255, 255, 255, 255);
ImU32 TEXT     = IM_COL32(0, 0, 0, 255);
ImU32 GRAYTEXT = IM_COL32(128, 128, 128, 255);
ImU32 SEL      = IM_COL32(0, 0, 128, 255);
ImU32 SEL_TEXT = IM_COL32(255, 255, 255, 255);
ImU32 FOLDER       = IM_COL32(255, 255, 128, 255);
ImU32 FOLDER_EDGE  = IM_COL32(128, 128, 0, 255);
ImU32 FOLDER_SHADE = IM_COL32(160, 160, 0, 255);
ImU32 ACCENT       = IM_COL32(0, 0, 255, 255);

Style Sty;

// Silver — the canonical Win95 "Windows Standard" scheme (matches the globals).
const Palette Palette95 = {
    IM_COL32(192,192,192,255), IM_COL32(223,223,223,255), IM_COL32(255,255,255,255),
    IM_COL32(128,128,128,255), IM_COL32(0,0,0,255),
    IM_COL32(0,0,128,255), IM_COL32(16,132,208,255), IM_COL32(128,128,128,255),
    IM_COL32(181,181,181,255), IM_COL32(255,255,255,255),
    IM_COL32(0,128,128,255), IM_COL32(255,255,255,255), IM_COL32(0,0,0,255),
    IM_COL32(128,128,128,255), IM_COL32(0,0,128,255), IM_COL32(255,255,255,255),
    IM_COL32(255,255,128,255), IM_COL32(128,128,0,255), IM_COL32(160,160,0,255),
    IM_COL32(0,0,255,255),
};

// NeXT Night — charcoal chrome, lighter-gray bevel highlights, black keyline,
// dark fields, light text. (Wired up in Phase 3; values tuned there.)
const Palette PaletteNight = {
    IM_COL32(60,60,60,255), IM_COL32(86,86,86,255), IM_COL32(110,110,110,255),
    IM_COL32(37,37,37,255), IM_COL32(0,0,0,255),
    IM_COL32(45,53,66,255), IM_COL32(74,92,116,255),   // active title: muted steel gradient (not loud blue)
    IM_COL32(42,42,42,255), IM_COL32(53,53,53,255), IM_COL32(224,224,224,255),
    IM_COL32(43,43,43,255), IM_COL32(27,27,27,255), IM_COL32(230,230,230,255),
    IM_COL32(106,106,106,255), IM_COL32(59,90,138,255), IM_COL32(255,255,255,255),
    IM_COL32(192,172,102,255), IM_COL32(110,98,48,255), IM_COL32(150,134,72,255),   // muted gold folder fill/edge/shade
    IM_COL32(126,146,176,255),                                                      // steel accent (view glyphs)
};

void ApplyScheme(const Palette& p, const Style& s) {
    FACE = p.face; LIGHT = p.light; HILIGHT = p.hilight; SHADOW = p.shadow; DKSHADOW = p.dkshadow;
    TITLE_ACT = p.title_act; TITLE_ACT2 = p.title_act2;
    TITLE_INACT = p.title_inact; TITLE_INACT2 = p.title_inact2; TITLE_TEXT = p.title_text;
    DESKTOP = p.desktop; WINBG = p.winbg; TEXT = p.text; GRAYTEXT = p.graytext;
    SEL = p.sel; SEL_TEXT = p.sel_text;
    FOLDER = p.folder; FOLDER_EDGE = p.folder_edge; FOLDER_SHADE = p.folder_shade;
    ACCENT = p.accent;
    Sty = s;
    ApplyStyle();   // re-derive ImGui colors from the new palette
}

} // namespace t95
