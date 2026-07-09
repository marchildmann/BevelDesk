#include "theme95/palette.h"
#include "theme95/style.h"

namespace t95 {

// The two scheme tables are the single source of truth for every chrome color.
// The live globals below bootstrap from Palette95 and ApplyScheme() swaps them,
// so each color is written in exactly one place per scheme.
const Palette Palette95 = {   // Silver — the canonical Win95 "Windows Standard"
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
// dark fields, light text, a muted-steel active caption and muted-gold folders.
const Palette PaletteNight = {
    IM_COL32(60,60,60,255), IM_COL32(86,86,86,255), IM_COL32(110,110,110,255),
    IM_COL32(37,37,37,255), IM_COL32(0,0,0,255),
    IM_COL32(45,53,66,255), IM_COL32(74,92,116,255),   // active title: muted steel gradient
    IM_COL32(42,42,42,255), IM_COL32(53,53,53,255), IM_COL32(224,224,224,255),
    IM_COL32(43,43,43,255), IM_COL32(27,27,27,255), IM_COL32(230,230,230,255),
    IM_COL32(106,106,106,255), IM_COL32(59,90,138,255), IM_COL32(255,255,255,255),
    IM_COL32(192,172,102,255), IM_COL32(110,98,48,255), IM_COL32(150,134,72,255),   // muted gold folder fill/edge/shade
    IM_COL32(126,146,176,255),                                                      // steel accent (view glyphs)
};

Style Sty;

// Live chrome colors read by every draw call. Bootstrapped to Silver *from*
// Palette95 (defined above) so the default look is byte-identical with no runtime
// setup; ApplyScheme() reassigns them to switch schemes.
ImU32 FACE     = Palette95.face;
ImU32 LIGHT    = Palette95.light;
ImU32 HILIGHT  = Palette95.hilight;
ImU32 SHADOW   = Palette95.shadow;
ImU32 DKSHADOW = Palette95.dkshadow;
ImU32 TITLE_ACT    = Palette95.title_act;
ImU32 TITLE_ACT2   = Palette95.title_act2;
ImU32 TITLE_INACT  = Palette95.title_inact;
ImU32 TITLE_INACT2 = Palette95.title_inact2;
ImU32 TITLE_TEXT   = Palette95.title_text;
ImU32 DESKTOP  = Palette95.desktop;
ImU32 WINBG    = Palette95.winbg;
ImU32 TEXT     = Palette95.text;
ImU32 GRAYTEXT = Palette95.graytext;
ImU32 SEL      = Palette95.sel;
ImU32 SEL_TEXT = Palette95.sel_text;
ImU32 FOLDER       = Palette95.folder;
ImU32 FOLDER_EDGE  = Palette95.folder_edge;
ImU32 FOLDER_SHADE = Palette95.folder_shade;
ImU32 ACCENT       = Palette95.accent;

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
