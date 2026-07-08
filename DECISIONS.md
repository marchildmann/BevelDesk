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
9. **Dependencies pinned**: GLFW 3.4, ImGui v1.91.9b (pre-1.92 font rework)
   via release tarballs; stb via shallow master clone (stb has no releases).
10. **Start menu items** other than "Shut Down…" (which exits the app) are
    decorative and just close the menu.
11. **imgui.ini disabled** (`io.IniFilename = nullptr`) so window layout is
    deterministic for screenshots.
12. **Window title of `/` is "My Computer"**-less: titles show the folder
    name (or the full path at the root). Simpler and predictable.
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
