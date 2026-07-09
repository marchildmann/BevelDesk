# Contributing to BevelDesk

Thanks for looking under the hood. BevelDesk is a Windows 95–style desktop
drawn entirely with Dear ImGui's `ImDrawList` API — no stock ImGui widgets, no
image assets. That constraint shapes everything below. This guide is the
distilled, hard-won knowledge from building it; most of these notes cost a real
debugging session to learn.

## Build, run, verify

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # once; FetchContent pulls GLFW/ImGui/stb
cmake --build build -j8                          # also assembles build/BevelDesk.app (macOS)
./build/beveldesk                                # windowed 1024x768 (default)
./build/beveldesk --fullscreen | --borderless | --windowed WxH
./build/beveldesk --screenshot [out.png]         # headless: render 10 frames -> PNG -> exit
./build/beveldesk --screenshot s.png --demo start|max|nav|icons|shutdown|dos|sysmenu|about|run|display
```

**Verification is two-tier, and the second tier is not optional:**

1. **Pixels** — run `--screenshot` with a `--demo` state that exercises your
   change, open the PNG, and compare against the authenticity rules below. For
   chrome details, crop-zoom the exact region 8–9× and read individual pixels;
   bugs are almost always a 1px color/position mismatch between two
   independently-drawn primitives. Log discrepancies in `CRITIQUE.md`.
2. **Interaction** — screenshots *cannot* catch input bugs (the custom
   scrollbar once shipped with three stacked ones: an ungrabbable thumb, a
   page-jump on grab, and half-speed dragging, none visible in a still). After
   touching any widget, actually run the app and click it.

To add a `--demo` QA state, add a branch in `main.cpp`; the screenshot fires at
frame 9 (later for the pty demo, which needs real time to start a shell).

## Architecture (respect the seams)

```
src/app/        model only, no drawing — AppState, ExplorerWin, DosWin, TermGrid
src/theme95/    the ENTIRE visual language, app-agnostic and reusable:
                palette.* (Palette/Style schemes + live color globals +
                ApplyScheme), bevel.*, widgets.*, window.* (chrome + system
                menu), style.*, icons95.*  (umbrella: theme95.h)
src/shell/      the desktop environment, one file per surface: desktop, taskbar
                (+ Start menu), explorer, dosprompt, shutdown, displayprops,
                about, run
src/platform/   OS glue: font discovery, pty (forkpty), GL textures, screenshots
```

- **A new program** (e.g. Notepad, Minesweeper): add a `shell/foo.{h,cpp}` that
  draws inside `t95::BeginWindow95(Chrome&)`, add its window struct to `app.h`,
  and register a taskbar entry + a Start-menu item. `shutdown.cpp` (a dialog)
  and `dosprompt.cpp` (a full window) are the templates.
- **A new mood** (like NeXT Night): it is *not* a new directory or an `ITheme`.
  Every draw call reads the live `t95::` color globals (`FACE`, `TITLE_ACT`, …),
  so a scheme is just a `Palette` table + a small `Style` (currently one
  `chiseled` bevel flag). Add a `PaletteFoo` (bootstrap the Silver globals from
  it if it should be the default), give it a `Style`, list it in
  `displayprops.cpp`'s Appearance list, and `ApplyScheme` swaps everything live.
  **Nothing in `shell/` may hard-code a chrome color or know how a bevel is
  drawn** — that's the seam that makes re-theming possible. Guardrail: the
  Silver scheme must stay byte-identical (golden `--demo` screenshots) — a new
  mood is purely additive.
- All drawing is `ImDrawList` on `InvisibleButton`s. Icons are procedural.
  Anti-aliasing is globally off so pixels stay crisp.

## Windows 95 authenticity rules

Checked against the research in `PLAN.md`. Consult `DECISIONS.md` before
"fixing" anything that looks odd — several choices are deliberate.

- **Palette:** face `#C0C0C0`, 3DLight `#DFDFDF`, shadow `#808080`, dark-shadow
  `#000000`, active caption `#000080`, desktop `#008080`. (Not Win98's
  `#D4D0C8`.) The caption gradient to `#1084D0` is a deliberate Win98 nod.
- **Bevels** are 2px = two 1px edges, light from the top-left. Raised button /
  pressed / sunken field / window frame each use a *different* edge-color
  order — see `theme95/bevel.h`.
- **Metrics:** caption 18px, caption buttons 16×14, window border 4px, taskbar
  28px, scrollbars 16px, list rows 17px. Menus cast **no** drop shadow (that's
  a Windows 3.1 / 2000 trait).

## Dear ImGui gotchas (each cost a debugging round)

1. **Overlapping `InvisibleButton`s:** the item submitted *first* claims the
   click and blocks hover on later items until release. Submit the small,
   precise hitbox (a scrollbar thumb) *before* the big one (the track).
2. **`SetScrollY` applies at the next frame's `Begin`.** A same-frame
   `GetScrollY` readback returns the stale value — never push-then-read in one
   frame (see the `scroll_pushed` guard in `explorer.cpp`).
3. **Global `ItemSpacing.y` pads stacked items:** N buttons of height H occupy
   `N*(H+spacing)`, not `N*H`. Any hand-computed content height / scroll math
   silently desyncs. Push `ItemSpacing (0,0)` around manually-measured stacks.
4. **Zoom & the mouse:** zoom shrinks `io.DisplaySize` by Z and sets
   `DisplayFramebufferScale = ratio*Z`. Input is event-based, so the GLFW
   backend queues the raw cursor position and `NewFrame` overwrites any
   `io.MousePos` you set beforehand (looks fine in screenshots, breaks live).
   Feed a corrected `io.AddMousePosEvent(cursor/Z)` *after* the backend, gated
   on `GLFW_HOVERED`.
5. **Adjacent hand-drawn borders must share edge weight and color** or a 1px
   seam shows. Two touching beveled shapes (tab↔page) must use the same border
   recipe; let the raised one overdraw the shared border so the fills merge.
6. Topmost surfaces (taskbar, menus) call `ImGui::BringWindowToDisplayFront`
   every frame; the desktop uses `NoBringToFrontOnFocus`.
7. **After any ImGui version bump, `rm -rf build`** — stale object files
   silently hide API breaks. (The 1.91→1.92 bump removed `ImFont::FontSize`
   and the OpenGL3 `*FontsTexture` calls, and made a mid-loop
   `io.Fonts->Clear()` crash; dynamic fonts rebake on their own.)

## Fonts

Probed from the system at runtime (`platform.cpp`): Tahoma / Microsoft Sans
Serif / DejaVu, plus bold and a monospace for the terminal. No font files ship
with the native build. The built-in ProggyClean is a last-resort fallback only
— it's monospaced, so it reads wrong for a proportional UI.

## MS-DOS Prompt / terminal

`platform/pty.*` `forkpty`s the user's `$SHELL` (POSIX only). `app/term.*` is a
VT100/xterm *subset* — solid for interactive shells, imperfect for full-screen
TUIs (no scroll regions or alt screen). It must answer DA (`ESC[c`) and DSR
(`ESC[6n`) or shells hang. Pump the pty every frame even while minimized, or a
full output buffer blocks the child. `Read()==0` means the shell exited → close
the window.

## WebAssembly build

The [live demo](https://marchildmann.github.io/BevelDesk/) is the same source
via Emscripten (all behind `#ifdef __EMSCRIPTEN__`). `web/` holds the HTML
shell, bundled DejaVu fonts, and a mock filesystem.

```sh
emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release && cmake --build build-web -j
python3 -m http.server -d build-web 8000   # open http://localhost:8000/index.html
```

Gotchas: the main loop must be an `emscripten_set_main_loop` callback; call
`ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas")` or the framebuffer
is 0 and the canvas stays blank; use a fixed `-sINITIAL_MEMORY` (a growable heap
is a resizable `ArrayBuffer`, which recent Chrome's `TextDecoder` rejects when
opening the font); the pty/DOS prompt no-ops in the browser.

## Project docs

- `PLAN.md` — the Win95 visual-language research (don't rewrite history)
- `DECISIONS.md` — every ambiguity resolved, numbered, with reasoning
- `CRITIQUE.md` — the compile → screenshot → inspect → fix iteration log
- `STATUS.md` — what works, known limitations, roadmap

PRs welcome. If you're fixing a rendering detail, a before/after screenshot (and
a real Win95 reference if you have one) makes review a lot faster.
