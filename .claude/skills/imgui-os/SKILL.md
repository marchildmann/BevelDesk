---
name: imgui-os
description: Working on ImGUI_OS — the Windows 95 desktop replica in Dear ImGui (C++/GLFW/OpenGL3). Use when building, running, debugging, or extending this app — new programs (Notepad, Minesweeper), new themes (NeXTSTEP), widget fixes, terminal/pty work, or authenticity questions. Covers the build/QA loop, the Theme95 drawing system, and hard-won ImGui gotchas (hitbox order, scroll readback lag, Retina fonts).
---

# ImGUI_OS — Windows 95 desktop in Dear ImGui

## Build, run, verify

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # once; FetchContent pulls GLFW/ImGui/stb
cmake --build build -j8                          # also assembles "build/BevelDesk.app"
./build/beveldesk                                    # windowed 1024x768 (default)
./build/beveldesk --fullscreen | --borderless | --windowed WxH
./build/beveldesk --screenshot [out.png]             # headless QA: 10 frames -> PNG -> exit
./build/beveldesk --screenshot s.png --demo start|max|nav|icons|shutdown|dos|sysmenu
```

**The verification loop is mandatory and two-tier:**
1. **Pixels**: run `--screenshot` (+ a `--demo` state that exercises your change),
   Read the PNG yourself, zoom with a crop tool if needed, compare against the
   conventions below. Log discrepancies in CRITIQUE.md, fix, repeat.
2. **Interaction**: screenshots CANNOT catch input bugs (a scrollbar shipped with
   three stacked ones). After changing any widget, trace the two gotchas under
   "ImGui gotchas" by hand, then relaunch for the user to play-test:
   `pkill -f build/beveldesk; ./build/beveldesk &` (run_in_background).

New QA states: add a `--demo <name>` branch in `main.cpp` — pre-arrange app
state (or inject at frame N inside the loop), screenshot fires at frame 9
(frame 140 + 10ms/frame sleep for pty demos that need real time).

## Architecture (respect the seams)

```
src/app/        model only, no drawing (AppState, ExplorerWin, DosWin, TermGrid)
src/theme95/    the ENTIRE visual language — palette.h, bevel.*, widgets.*,
                window.* (chrome+system menu), style.*, icons95.* ; umbrella theme95.h
src/shell/      one file per surface: desktop, taskbar(+start menu), explorer,
                dosprompt, shutdown
src/platform/   fonts, pty (forkpty), GL textures, screenshots
```

- **New program** (Notepad, Minesweeper…): new `shell/foo.{h,cpp}` drawing inside
  `t95::BeginWindow95(Chrome&)`; add window struct to app.h; register a task-button
  entry in taskbar.cpp's `entries` vector and a Start-menu item. `shutdown.cpp`
  (dialog, ~90 lines) and `dosprompt.cpp` (full window) are the templates.
- **New theme**: sibling of `theme95/` implementing the same surface (palette /
  bevels / chrome). Nothing in `shell/` may know how a bevel is drawn.
- ALL drawing is ImDrawList on InvisibleButtons — never stock ImGui widgets.
  Icons are procedural (no image assets). AA is globally off (crisp pixels).

## Win95 authenticity rules (checked against research in PLAN.md)

- Palette: face #C0C0C0, 3DLight #DFDFDF, shadow #808080, dkshadow #000000,
  caption #000080 (solid, no gradient), desktop #008080. NOT Win98's #D4D0C8.
- Bevels are 2px, two 1px edges, light from top-left; button / pressed / sunken
  field / window frame each have DIFFERENT edge color orders — see bevel.h.
- Metrics: caption 18px, caption buttons 16x14, window border 4px, taskbar 28px,
  scrollbars 16px. Menus have NO drop shadow (that's Win3.1/Win2000).
- Consult DECISIONS.md before "fixing" anything odd-looking — several choices are
  deliberate (hidden dotfiles, KB sizes, fixed 80x25 DOS box, shadowless menus).

## ImGui gotchas (each cost a debugging round)

1. **Overlapping InvisibleButtons**: the item submitted FIRST claims the mouse
   press and blocks later items' hover until release. Submit the small/precise
   hitbox (thumb) BEFORE the big one (track).
2. **`SetScrollY` applies at the NEXT frame's `Begin`** — a same-frame
   `GetScrollY` readback returns the stale value. Never push-then-readback in
   one frame (see `scroll_pushed` guard in explorer.cpp).
2b. **Global `ItemSpacing.y` pads stacked items**: N buttons of height H
   occupy N*(H+spacing), not N*H — any hand-computed content_h/scroll math
   silently desyncs (scrollbar appeared late, under-scrolled). Push
   `ItemSpacing (0,0)` around manually-measured item stacks; 17px unpadded
   rows are also the authentic Win95 list density.
3. **Retina / zoom**: interactive mode uses native framebuffer + font
   `RasterizerDensity` = pixel ratio; `--screenshot` forces 1x
   (`GLFW_COCOA_RETINA_FRAMEBUFFER=FALSE`) for deterministic 1024x768 PNGs.
   Zoom = shrink `io.DisplaySize` by Z, set `DisplayFramebufferScale` =
   ratio*Z, divide `io.MousePos` by Z (all BEFORE `NewFrame`). On **1.92**
   fonts rebake crisply from FramebufferScale automatically — do NOT rebuild
   the atlas (a mid-loop `io.Fonts->Clear()` SIGSEGVs). `ImFont::FontSize`
   and `ImGui_ImplOpenGL3_*FontsTexture` are gone in 1.92 — use
   `ImGui::GetFontSize()`. After any ImGui tag bump, `rm -rf build` (stale
   .o files silently hide API breaks).
4. Fonts are system-probed at runtime (Tahoma/micross/DejaVu + bold + mono for
   the terminal) in platform.cpp — never ship font files, never use default
   ProggyClean for UI (monospaced = wrong spacing).
5. Topmost surfaces (taskbar, menus) call `ImGui::BringWindowToDisplayFront`
   (imgui_internal.h) every frame; desktop uses `NoBringToFrontOnFocus`.
6. Custom chrome text: `dl->AddText` with explicit clip ImVec4; bold via
   `t95::AddTextBold` (real bold font or 1px overdraw fallback).
7. **Adjacent hand-drawn borders must share edge WEIGHT and COLOR, or the
   1px seam shows.** When two beveled shapes touch (tab↔page, groupbox↔frame),
   don't mix a 2px `WindowFrame` (outer #DFDFDF / inner #FFFFFF) against a
   1px pure-white edge — at the join one "white" line sits at #DFDFDF and the
   other at #FFFFFF and they visibly step. Fix: give both the SAME border
   recipe (Win95 tab pages use 1px HILIGHT top/left + 2px SHADOW/DKSHADOW
   bottom/right — NOT WindowFrame), and let the raised element overdraw the
   shared border along its own width so the fills merge (FACE→FACE, no seam).

## Debugging pixel-alignment gripes (the method that works)

When the user says "this border/line doesn't match," don't guess from the
full screenshot — **crop-zoom the exact junction 8–9x** with the stdlib PNG
tool (scratchpad/pngtool.py: `<src> <dst> x y w h zoom`) and read the
individual pixels. The bug is almost always a 1px color/position mismatch
between two independently-drawn primitives (e.g. tab edge vs page edge). Then
compare against a real Win95 reference the user provides, pixel row by pixel row.
Reference images beat memory for chrome this exact.

## MS-DOS Prompt / pty specifics

- `platform/pty.*` forkpty's `$SHELL -i` with TERM=xterm-16color; POSIX only.
- `app/term.*` is a VT100/xterm SUBSET (no scroll regions, no real alt screen)
  — shells are solid, full-screen TUIs render imperfectly. It must answer DA
  (`ESC[c`) and DSR (`ESC[6n`) or shells hang; DECCKM switches arrow encoding.
- Pump the pty EVERY frame even when minimized or the shell blocks on a full
  buffer. `Read()==0` means the shell exited -> close the window (authentic).
- Ctrl+Q / Ctrl+C etc. must reach a focused terminal — global shortcuts check
  `dos_focused` first.

## Docs contract

PLAN.md = research (don't rewrite history). DECISIONS.md = numbered choices —
append when resolving ambiguity. CRITIQUE.md = iteration log — append per
visual-QA round. README.md + STATUS.md = keep current with every feature.
