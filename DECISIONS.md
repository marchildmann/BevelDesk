# DECISIONS

Ambiguities resolved autonomously during the mission.

1. **Win95 palette, not Win98/2000.** Sources disagree (#D4D0C8 vs #C0C0C0):
   #D4D0C8 is the Windows 98/2000 default. Windows 95 "Windows Standard" used
   silver `#C0C0C0` chrome, navy `#000080` active captions (solid, no
   gradient), teal `#008080` desktop. Chose the true 95 values.
2. **"My Computer" opens `/`** (filesystem root) — the closest analog of a
   drive list on macOS/Linux. "Recycle Bin" opens `~/.Trash` when it exists,
   otherwise the home directory.
3. **Hidden (dot) files are not listed** — matches Win95's default "don't
   show hidden files".
4. **File sizes in KB, rounded up, comma-separated** ("1,234KB") like the
   Win95 Details view; folders show no size. No "Modified" column: portable
   `file_time -> time_t` conversion is C++20/platform-dependent, and three
   columns (Name/Size/Type) are enough for the look.
5. **Font** (revised after user feedback): ProggyClean is monospaced and
   reads too airy vs. the proportional MS Sans Serif. The app now probes for
   the closest proportional system UI face at 12px — macOS: Tahoma → Verdana
   → Arial; Windows: micross.ttf (actual Microsoft Sans Serif) → Tahoma;
   Linux: DejaVu Sans → Liberation Sans — plus the matching Bold for
   captions, Start and task buttons. Falls back to ProggyClean (+1px
   overdraw bold) if none exist. No files are shipped; system fonts only.
6. **Menu bar** (File Edit View Help) is decorative (drawn, hover highlight,
   no dropdowns) — the mission requires navigation via toolbar/list, not
   menus.
7. **Retina/HiDPI** (revised after user feedback — text looked soft):
   interactive mode renders at native Retina resolution with the font
   rasterized at the monitor's content scale (`RasterizerDensity`); logical
   metrics stay 96-dpi-style, so 1 logical px = 2 crisp device px.
   `--screenshot` mode keeps `GLFW_COCOA_RETINA_FRAMEBUFFER=FALSE` so QA
   PNGs stay a deterministic 1024x768.
8. **Screenshot mode** uses a hidden 1024×768 window, renders 10 frames,
   reads the back buffer. Pre-opens one Explorer window (at `/`) so the
   screenshot demonstrates window chrome + listview.
9. **Dependencies pinned**: GLFW 3.4, ImGui v1.92.8 (dynamic-font system —
   see #13d; originally pinned to pre-1.92 v1.91.9b, upgraded for crisp zoom)
   via release tarballs; stb via shallow master clone (stb has no releases).
10. **Start menu items** other than "Shut Down…" (which exits the app) are
    decorative and just close the menu.
11. **imgui.ini disabled** (`io.IniFilename = nullptr`) so window layout is
    deterministic for screenshots.
12. **Window title of `/` is "My Computer"**-less: titles show the folder
    name (or the full path at the root). Simpler and predictable.
13d. **Zoom via projection; upgraded to ImGui 1.92.8 for dynamic fonts**
    (prompted by @ocornut's feedback). Zoom shrinks io.DisplaySize by Z (the
    fixed-pixel hand-drawn UI magnifies), sets DisplayFramebufferScale =
    pixel_ratio*Z, and divides MousePos by Z. On **1.92** the font atlas
    rebakes glyphs on demand at the density DisplayFramebufferScale implies,
    so text stays crisp with NO manual atlas rebuild (1.91 needed a
    Clear()+re-load each zoom step). Input: Ctrl/Cmd + mouse-wheel
    (layout-independent) or +/-/0.
    Migration notes (1.91.9b → 1.92.8): `ImFont::FontSize` field was removed
    (use `ImGui::GetFontSize()`); `ImGui_ImplOpenGL3_Create/DestroyFontsTexture`
    are gone (dynamic-texture backend manages uploads); a mid-loop
    `io.Fonts->Clear()` now SIGSEGVs — don't rebuild, rely on dynamic rebake.
    Watch for stale object files masking API breaks: do a clean `rm -rf build`
    after the tag bump or you'll compile main.cpp against the old headers.
13c. **Windowed by default** (revised twice per user feedback). macOS rounds
    the corners of titled windows, which breaks the Win95 illusion — but a
    surprise fullscreen takeover on launch is worse. Default is a normal
    1024x768 window; `--fullscreen` gives total immersion, `--borderless`
    an undecorated square-cornered window whose "macOS chrome" is the Win95
    taskbar itself (empty-area drag moves the host window, Cmd+M minimizes).
    Cmd/Ctrl+Q always exits (suppressed while a DOS prompt has focus —
    Ctrl+Q is XON flow control there).
13b. **No menu drop shadows.** Considered a hard 1px black offset shadow
    under the Start/system menus to make them read as floating — rejected:
    hard menu shadows are a Windows 3.1-ism, and soft shadows arrived with
    Windows 2000. Stock Win95 menus are flat raised panels with no shadow;
    we stay faithful. The DOS-window system menu does gain the (grayed)
    "Properties" entry real console windows had.
13a. **MS-DOS Prompt scope**: authentic fixed 80×25 grid (no resize, no
    maximize — real DOS windows maximized to full-screen text mode, which
    doesn't translate). TERM=xterm-16color; the emulator implements the
    subset an interactive shell needs (cursor/erase/SGR/OSC/DECCKM/DA/DSR)
    and deliberately skips scroll regions and a separate alt screen. The
    pty is pumped even while minimized so the shell never blocks on a full
    buffer. Exit of the shell closes the window, like the original.
13. **Component layout** (added when the project outgrew flat files):
    `app/` model, `theme95/` visual language (palette/bevel/widgets/window/
    style/icons), `shell/` desktop surfaces (desktop/taskbar/explorer/
    shutdown), `platform/` OS glue (fonts/GL/screenshot). Headers include
    via component-qualified paths ("theme95/bevel.h"); `theme95/theme95.h`
    is an umbrella. Verified pixel-identical rendering after the move.

14. **Win98 gradient captions** (user request). Win95 used a solid navy
    caption; we switched to the iconic Windows 98 horizontal gradient
    (navy #000080 → #1084D0 active, gray → light gray inactive) via
    `AddRectFilledMultiColor`. A deliberate departure from strict Win95 for
    the nicer look.
15. **WebAssembly via Emscripten.** Same source, `#ifdef __EMSCRIPTEN__`
    branches: main loop → `emscripten_set_main_loop`; WebGL2/GLES3 shaders;
    GLFW from `-sUSE_GLFW=3`; `ImGui_ImplGlfw_InstallEmscriptenCallbacks` for
    canvas sizing (without it framebuffer is 0 → blank). Fonts (DejaVu) and a
    mock filesystem are `--preload-file`'d into MEMFS. Fixed 256MB heap, NOT
    `ALLOW_MEMORY_GROWTH`: a growable heap is a resizable ArrayBuffer, which
    recent Chrome's `TextDecoder` rejects (crashed opening the font). The
    pty/DOS prompt no-ops on web. Deployed to GitHub Pages via `pages.yml`.

16. **Two moods, not a theme engine** (the "NeXT Night" mission). BevelDesk
    gains a dark scheme *inspired by* NeXTSTEP's chiseled chrome — its own
    design language, NOT a NeXTSTEP clone and explicitly NOT a Dock (the Win95
    taskbar already launches + task-switches compactly; a Dock would be a
    second launcher for the same job). Win95 and NeXTSTEP share one visual
    philosophy (3D bevels, light from top-left, per-window chrome), so a dark
    mode is a *palette + bevel-style* variation, not a shell rewrite. We
    deliberately stopped at two curated moods rather than building a general
    ITheme abstraction.
16a. **The theming mechanism** (kept lightweight on purpose). The chrome colors
    that were `constexpr` became reassignable `extern ImU32` globals in
    `theme95/palette.*`; every draw call reads them unchanged. Two `Palette`
    tables (`Palette95`, `PaletteNight`) plus a small `Style` (currently one
    `chiseled` flag) are the whole surface; `ApplyScheme(const Palette&, const
    Style&)` swaps them and re-derives the ImGui style. The live globals
    *bootstrap from* `Palette95` (defined first in the same TU) so Silver needs
    no runtime setup and there is one source of truth per color. Semantic,
    non-chrome colors (BLACK/GREEN/NAVY) stay `constexpr`.
16b. **Win95 stays byte-identical** (Phase 2 guardrail). Converting the palette
    to runtime globals is behavior-preserving: golden screenshots of every
    `--demo` state diff to zero against Silver except the live taskbar clock.
    The one folder-icon shade line that was a distinct olive (`160,160,0`, not
    the `128,128,0` outline) got its own themeable slot (`FOLDER_SHADE`) rather
    than collapsing into `FOLDER_EDGE`, precisely to keep Silver identical.
16c. **NeXT Night is BevelDesk's own dark, not "inverted Win95".** Charcoal
    chrome (#3C3C3C face), chiseled bevels (a black keyline + a 1px 3D edge,
    vs Win95's two-edge bevel), a muted-steel active caption (so focus reads
    without a loud gradient), muted-gold folders and a steel view-mode accent
    (the neon VGA yellow/blue looked garish on charcoal), and light text.
    Icon colors (folders, view glyphs, the small accent) are themeable so they
    are calm in Night without touching Silver.
16d. **Runtime toggle lives in Display Properties ▸ Appearance** (the natural
    home; the four-tab strip already existed). Selecting a scheme in the
    Appearance list live-applies via `ApplyScheme`; OK commits, Cancel/Esc/X
    revert both the background color and the scheme to their pre-dialog state.
    Switching scheme moves the desktop to the new scheme's *default* background
    only if it was still on the old default — a background the user
    deliberately picked is preserved across the switch. `--night` boots
    straight into the dark scheme.
