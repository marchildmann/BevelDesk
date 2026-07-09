# PLAN — "NeXT Night": BevelDesk's dark mode

Phase 1 design for the mission in CLAUDE.md. BevelDesk gains a second, dark,
chiseled look — **NeXT Night** — switchable at runtime, on the exact Win95
structure we already have (taskbar + Start, per-window chrome, Explorer). This
is *inspired by* NeXTSTEP's dark chiseled widgets and grayscale chrome, NOT a
NeXT clone. No Dock (the taskbar already launches + switches; see CLAUDE.md).

## The core idea: the palette becomes reassignable, not the code

Today `theme95/palette.h` holds `constexpr ImU32 FACE = …` etc., read all over
`bevel.*`, `widgets.*`, `window.*`, `icons95.*`, `shell/*`. The whole theming
mechanism is: **make those constants mutable globals and reassign them on a
switch.** Every existing call site (`dl->AddRectFilled(…, FACE)`) keeps working
unchanged — the value it reads just depends on the active scheme.

```cpp
// palette.h  — was: constexpr ImU32 FACE = IM_COL32(192,192,192,255);
namespace t95 {
extern ImU32 FACE, LIGHT, HILIGHT, SHADOW, DKSHADOW, TITLE_ACT, TITLE_ACT2,
             TITLE_INACT, TITLE_INACT2, TITLE_TEXT, WINBG, TEXT, GRAYTEXT,
             SEL, SEL_TEXT, DESKTOP_DEFAULT;
// BLACK and the VGA accents (RED/GREEN/BLUE/YELLOW/NAVY/DKYELLOW) stay
// constexpr — they're semantic, not chrome, and don't change per scheme.

struct Palette {                       // one value per themeable chrome color
    ImU32 face, light, hilight, shadow, dkshadow;
    ImU32 title_act, title_act2, title_inact, title_inact2, title_text;
    ImU32 winbg, text, graytext, sel, sel_text, desktop_default;
};
struct Style { bool chiseled = false; };   // NeXT keyline; room to grow

extern Palette Pal; extern Style Sty;
void ApplyScheme(const Palette& p, const Style& s);  // reassign globals + re-ApplyStyle
}
```

`ApplyScheme` copies the palette's fields into the loose globals (`FACE = p.face`
…), stores `Sty`, and re-runs `ApplyStyle()` (which derives a few ImGui colors
from `FACE`/`WINBG`/`SEL`). Switching schemes = one `ApplyScheme` call.

Why loose globals *and* a `Palette` struct: the globals give zero-churn call
sites; the struct gives a clean, copyable definition of each scheme and makes
"add a third mood" literally "fill one more `Palette`."

## The two schemes

**Silver (Win95) — must stay byte-identical to today.** Exactly the current
values: face `#C0C0C0`, light `#DFDFDF`, hilight `#FFFFFF`, shadow `#808080`,
dkshadow `#000000`, title `#000080`→`#1084D0`, inactive `#808080`→`#B5B5B5`,
title_text white, winbg white, text black, graytext `#808080`, sel `#000080`,
sel_text white, desktop_default `#008080`. `Style{ chiseled=false }`.

**NeXT Night — proposed (tune in Phase 3 against zoomed screenshots):**
| role | hex | note |
|------|-----|------|
| face | `#3C3C3C` | charcoal chrome |
| hilight | `#6E6E6E` | top-left bevel — a *lighter gray*, not white |
| light | `#565656` | inner top-left |
| shadow | `#252525` | inner bottom-right |
| dkshadow | `#000000` | outer bottom-right + the chiseled keyline |
| title_act / title_act2 | `#1E1E1E` → `#3A3A3A` | dark caption, subtle gradient |
| title_inact / title_inact2 | `#2A2A2A` → `#353535` | |
| title_text | `#EDEDED` | light caption text |
| winbg | `#1B1B1B` | dark list/edit fields |
| text | `#E6E6E6` | light body text |
| graytext | `#6A6A6A` | disabled |
| sel | `#3B5A8A` | desaturated blue selection |
| sel_text | `#FFFFFF` | |
| desktop_default | `#2B2B2B` | dark desktop (user can still recolor) |
`Style{ chiseled=true }`.

Contrast/read must be verified with the crop-zoom method — the bevel has to
still read as raised/sunken in the dark palette (that's the risk in dark mode).

## The one bevel variation: the chiseled keyline

NeXT's signature is a **1px black keyline around** the beveled element (all four
sides), with the light/dark bevel *inside* it. Win95 has no such all-around
keyline. So `Style::chiseled` is the single behavioral knob: when true,
`BevelRaised`/`BevelPressed`/`SunkenField` draw a black `AddRect` outline first,
then the normal 2px bevel inset by 1px. When false (Silver), they're exactly as
today. This keeps Win95 untouched and gives Night its chiseled character with a
handful of lines in `bevel.cpp`.

## Runtime switch: Display Properties → Appearance

The Display Properties dialog already renders four tabs (Background, Screen
Saver, **Appearance**, Settings) — only Background is wired. Wire **Appearance**:
a "Scheme" chooser (two labelled swatches / radios: *BevelDesk Silver* and
*BevelDesk Night*) that calls `ApplyScheme(...)` and persists the choice in
`AppState` (session-only for now; a config file is a later nicety). Optionally a
Start-menu shortcut. No font change — Tahoma reads fine dark, keeping scope tight.

## Out of scope for now (note, don't do)

- **Icons.** The yellow folders / colored logo use the constexpr VGA accents and
  will look slightly bright on dark chrome. Acceptable for v1; a later pass can
  give icons a per-scheme variant. The desktop icon *label* text should read
  `TEXT` so it flips automatically.
- No Dock. No NeXT vertical menus. No NeXT left-side scrollers. No fidelity
  chase — this is BevelDesk's own dark mood.

## Migration order (Phase 2 = behavior-preserving; Win95 IDENTICAL)

1. Capture golden `--screenshot`s of every `--demo` state into `golden/`.
2. Convert `palette.h` constants → `extern` globals; add `palette.cpp` defining
   them at the Silver values + the `Palette`/`Style` structs + `ApplyScheme`.
   Call `ApplyScheme(Palette95, Style95)` once at startup.
3. Re-screenshot every demo, **diff against golden — must be byte-identical.**
   Fix any drift before moving on. (Watch: `ApplyStyle` deriving ImGui colors;
   anything that captured a palette value by copy at init.)
4. Only then (Phase 3) add `PaletteNight` + the `chiseled` bevel path and tune.
5. Phase 4: wire the Appearance tab; verify BOTH schemes live (screenshots +
   real interaction); update README/STATUS/DECISIONS/CONTRIBUTING.

## Guardrails (from CLAUDE.md)

Win95/Silver byte-identical after Phase 2 (golden diff). At most two curated
moods — no general engine, no Dock, no NeXT fidelity. Don't break the 3 native
builds or WASM. All chrome stays ImDrawList / procedural / AA-off. Two-tier
verify after every change. Honest commits, no Co-Authored-By.
