# STATUS

## What works

- Clean build from scratch on macOS (AppleClang, CMake FetchContent pulls
  GLFW 3.4 / ImGui 1.92.8 / stb; zero compile errors, one upstream stb
  `sprintf` deprecation warning). CMake is cross-platform (macOS / Linux /
  Windows link blocks present) and CI builds all three natives + WASM.
- Win95 desktop: teal background, procedurally drawn My Computer / Recycle
  Bin icons with select + double-click-to-open.
- Custom window chrome (Theme95): 4px window frame with the correct
  frame-edge colors, 18px navy/gray caption with bold title, working
  minimize / maximize-restore / close buttons (restore returns to the
  pre-maximize geometry), caption drag, double-click-to-maximize,
  resizable windows. System-menu box on the caption icon: single click
  opens the Restore/Move/Size/Minimize/Maximize/Close menu (Move/Size are
  authentically listed but grayed), double-click closes the window.
- Explorer: real filesystem via `std::filesystem`, folders-first sorted
  Details view (Name/Size/Type), KB sizes right-aligned, Up button, address
  well, custom Win95 scrollbar (arrows, dithered track, proportional thumb,
  paging, wheel), status bar with object count + total size, multiple
  windows at once, per-window state.
- Taskbar: Start button (pressed while menu open), Start menu (Shut Down…
  exits; sidebar band, separator, submenu arrows), per-window task buttons
  with minimize/restore/focus + pressed/lit active state, live clock.
- `--screenshot [path]` headless-friendly capture (hidden window, 10 frames,
  back-buffer read, PNG via stb) + `--demo start|max|nav` QA states.
- Visual fidelity was verified against research down to exact RGB values of
  every bevel layer (see CRITIQUE.md iterations 1–5).

## Added after the first release pass

- Proportional system UI font (Tahoma/micross/DejaVu + real bold) replacing
  monospaced ProggyClean.
- Shut Down Windows dialog with the dithered screen fade (Yes = quit,
  Restart = fresh session, No/Esc/X = cancel).
- Sortable column headers (Name/Size/Type, asc/desc, folders first).
- Large Icons view + toolbar view-mode toggle buttons.
- Native-Retina rendering with 2x font rasterization (crisp text on HiDPI).
- `build/BevelDesk.app` bundle — double-click launch from Finder without a
  Terminal window (macOS).
- **MS-DOS Prompt**: real interactive shell ($SHELL) on a forkpty pty,
  80x25 grid, VT100/xterm-subset emulator (app/term.*) with 16-color SGR +
  256-color mapping, scrollback, OSC titles, DECCKM, DA/DSR replies;
  Start-menu launch, taskbar integration, multiple instances; the window
  closes itself when the shell exits.
- **NeXT Night dark scheme**: a second `Palette` + `Style` (charcoal chrome,
  chiseled black-keyline bevels, muted-steel active caption, muted-gold
  folders, steel view-mode accent, light text) that every draw call reads via
  the live `t95::` color globals. Runtime toggle in Display Properties ▸
  Appearance (live-apply, OK/Cancel/Apply revert of both color and scheme);
  `--night` boots into it. Win95 (Silver) verified byte-identical vs golden
  after the palette became runtime-swappable.

## Known limitations

- Font falls back to ProggyClean (monospaced, 1px-overdraw bold) only if no
  system UI font is found; glyphs are grayscale-antialiased, not the exact
  MS Sans Serif bitmaps.
- Every Start menu item now does something (Programs→DOS, Documents/Find→
  Explorer, Settings→Display Properties, Help→About, Run…→Run box, Shut Down).
  The menu bar (File/Edit/View/Help) is still decorative; no context menus.
  The chosen desktop color / zoom level are session-only (no config file yet).
- UI zoom (Ctrl/Cmd + wheel or +/-/0, 1x–3x): the logical display is shrunk
  so the fixed-pixel UI magnifies and the cursor is mapped back to logical
  space. On ImGui 1.92 dynamic fonts rebake crisply from the framebuffer
  scale automatically — no manual atlas rebuild needed.
- MS-DOS Prompt: POSIX only (Windows needs ConPTY); no scroll regions or
  full alt-screen, so full-screen TUIs (vim, htop) render imperfectly —
  line-oriented shell work is solid. No clipboard paste yet; fixed 80x25.
- No file operations (open/copy/delete) — browsing only; symlinks are listed
  as plain files ("0KB File"); listings capped at 2000 entries; hidden
  dotfiles are skipped by design.
- Maximized windows keep their 4px border visible (real Win95 clips it
  offscreen).
- Single-monitor assumption; fixed 96-dpi logical metrics (content scale is
  read once at startup, not on monitor changes mid-session).
- `--screenshot` needs a windowing session (hidden GLFW window) — it is CI
  friendly on macOS/Windows, on Linux it requires X/Wayland (use Xvfb).

## Suggested next steps

- **A third mood**: the Palette/Style seam is proven (Silver + NeXT Night) —
  a new scheme is one more `Palette` table plus any `Style` flags, added to
  the Appearance list. Per-scheme *desktop patterns* (a stippled dark ground)
  and per-scheme icon sets would deepen a mood further.
- Persist the chosen scheme / desktop color / zoom to a config file (currently
  session-only).
- Real MS Sans Serif-like bitmap font embedded as a compiled-in header
  (pixel-exact glyphs at 8pt, no AA).
- Drive list for "My Computer"; context menus; renaming.
- Start menu submenus (Programs → launch multiple app windows: Notepad,
  Minesweeper…).
- Window minimize/restore animations ("zoom" rectangles).
