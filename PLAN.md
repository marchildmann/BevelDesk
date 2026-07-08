# PLAN — "ImGUI_OS": a Windows 95 desktop in Dear ImGui

Goal: a complete, compiling, runnable C++ app that renders an authentic-looking
Windows 95 desktop (teal wallpaper, icons, taskbar with Start button + live
clock) and working Explorer windows over the real filesystem, plus a
`--screenshot` mode for automated visual inspection.

## 1. Research: the Windows 95 visual language

### 1.1 System palette (the 20 static system colors that matter here)

Windows 95 default "Windows Standard" scheme (NOT the later Win98/2000
#D4D0C8 taupe — Win95 used pure silver #C0C0C0):

| Role                  | Hex       | Notes |
|-----------------------|-----------|-------|
| ButtonFace (3DFace)   | `#C0C0C0` | all chrome, dialogs, taskbar |
| ButtonHighlight       | `#FFFFFF` | top/left outer edge of raised elements |
| 3DLight               | `#DFDFDF` | top/left inner edge of raised elements |
| ButtonShadow          | `#808080` | bottom/right inner edge |
| ButtonDkShadow        | `#0A0A0A`/black | bottom/right outer edge |
| ActiveTitle           | `#000080` | navy title bar (solid — no gradient in 95) |
| InactiveTitle         | `#808080` | gray title bar |
| TitleText             | `#FFFFFF` | |
| Desktop (Background)  | `#008080` | the famous teal |
| Window                | `#FFFFFF` | edit fields, listviews |
| WindowText / MenuText | `#000000` | |
| Highlight             | `#000080` | selection background |
| HighlightText         | `#FFFFFF` | |
| GrayText              | `#808080` | disabled |
| Menu                  | `#C0C0C0` | |

VGA-16 accent colors for icons/logo: red `#FF0000`, green `#008000`/`#00FF00`,
blue `#0000FF`, yellow `#FFFF00`, dark yellow `#808000`, navy `#000080`,
teal `#008080`, plus black/white/grays.

### 1.2 The 3D bevel convention (DrawEdge semantics)

Light comes from the top-left. All bevels are 2px, built from two 1px
rectangular "edges", each with a distinct top-left (TL) and bottom-right (BR)
color. Verified against pixel-accurate recreations (98.css box-shadow stacks):

- **Raised button** (push buttons, caption buttons, taskbar buttons):
  outer TL `#FFFFFF`, outer BR black; inner TL `#DFDFDF`, inner BR `#808080`.
- **Pressed button**: outer TL black, outer BR `#FFFFFF`; inner TL `#808080`,
  inner BR `#DFDFDF`. Label shifts +1,+1.
- **Sunken field** (edit boxes, listviews, path bar): outer TL `#808080`,
  outer BR `#FFFFFF`; inner TL black, inner BR `#DFDFDF`; white fill.
- **Window frame** (note: NOT the same as a button!): outer TL `#DFDFDF`,
  outer BR black; inner TL `#FFFFFF`, inner BR `#808080`.
- **Thin sunken groove** (status bar cells, separators): 1px TL `#808080`,
  1px BR `#FFFFFF`.
- **Taskbar**: ButtonFace slab with a 1px `#FFFFFF` highlight line across its
  top edge.

### 1.3 Window chrome metrics (96 dpi defaults)

- Sizing border: 4px of chrome around the client (2px bevel + 2px face).
- Title bar (caption): **18px** tall, solid navy, small app icon (16×16) at
  left, bold white title, caption buttons right-aligned.
- Caption buttons: **16×14**, raised bevel, black glyphs; order
  `[minimize][maximize] gap [close]` with a 2px gap before close, inset 2px
  from top/right of the caption.
- Double-clicking the caption maximizes/restores.
- Maximized windows fill the work area (screen minus taskbar).

### 1.4 Taskbar + Start button anatomy

- Taskbar: full-width bar at bottom, ~28px tall, ButtonFace, 1px white top
  edge. Content vertically centered (22px tall buttons).
- Start button: raised bevel, Windows flag logo (2×2 red/green/blue/yellow
  panes) + bold "Start". ~54×22.
- One task button per open window, max ~160px wide, raised; the **active**
  window's button is drawn pressed/sunken with a lighter fill. Clicking
  toggles minimize/restore/focus.
- Clock in a thin-sunken well at the far right, `h:mm AM/PM`.
- Start menu: raised panel, dark-blue vertical sidebar band on the left,
  menu items (Programs, Documents, Settings, Find, Help, Run…, Shut Down…),
  navy highlight on hover.

### 1.5 Explorer (folder window) structure

Win95 "single folder" view (the My Computer / folder window):
- Menu bar (File Edit View Help) — we render it as static text row.
- Toolbar: flat buttons that raise on hover (COMCTL32 style); includes an
  "Up One Level" button; a path/address well (sunken, white).
- Listview in **Details** mode: sunken white field; column headers are raised
  gray buttons (Name, Size, Type); rows = 16-17px, small icon + name; sizes
  right-aligned, shown in KB (rounded up, comma-separated).
- Only the *name* cell highlights navy when selected.
- Status bar: thin-sunken wells — "N object(s)" and total size — plus a
  dotted resize grip bottom-right.
- Folders sort first, then files, case-insensitive.

### 1.6 Icons (procedural, DrawList only)

- **My Computer**: gray monitor (navy screen) on a horizontal desktop case
  with floppy slot + button, black outlines.
- **Recycle Bin**: gray waste basket — elliptical opening, tapered body with
  vertical shading stripes.
- Desktop icon cell: 32×32 icon centered, label below in white; selected
  label gets a navy background.
- Small 14px folder (yellow) / document (white page, folded corner) icons
  for list rows; mini computer icon for title bars.

## 2. Tech stack & build

- **Dear ImGui v1.91.9b** + **GLFW 3.4** + OpenGL 3.2 core (GLSL `#version
  150` — the macOS-safe combo), **stb_image_write** for PNG.
- All deps via CMake **FetchContent** (URL tarballs for speed); ImGui has no
  CMakeLists, so we compile its 4 core TUs + the glfw/opengl3 backends into a
  static lib. Backend uses its embedded GL loader; no GLAD needed.
- `GLFW_COCOA_RETINA_FRAMEBUFFER = FALSE` so framebuffer == window size:
  crisp 1:1 pixels (essential for the pixel-bevel look) and a simple
  screenshot path.
- C++17 (`std::filesystem`).

## 3. Architecture

```
src/
  theme95.h/.cpp   palette, Edge/bevel primitives, Button, ToolButton,
                   CaptionButton, BeginWindow95/EndWindow95 (custom chrome:
                   frame, navy caption, drag, min/max/close, maximize logic)
  icons95.h/.cpp   all procedural icons (DrawList primitives, AA off)
  app.h            AppState, ExplorerWin model
  explorer.cpp     directory scan (std::filesystem), Details listview,
                   toolbar (Up), path well, status bar
  desktop.cpp      teal desktop + icons, taskbar, Start menu, clock
  main.cpp         GLFW/ImGui boot, main loop, --screenshot mode
  stb_impl.cpp     STB_IMAGE_WRITE_IMPLEMENTATION
```

Key techniques:
- Every Win95 window is an ImGui window with `NoTitleBar`, zero padding/
  rounding/border; ALL chrome is custom DrawList work. Dragging = invisible
  button over the caption + `SetWindowPos`. `style.AntiAliasedLines/Fill =
  false` for pixel-crisp edges.
- Desktop uses `NoBringToFrontOnFocus` (always behind); taskbar + start menu
  use `BringWindowToDisplayFront` (imgui_internal.h) every frame (always on
  top).
- Explorer windows live in a `std::vector<ExplorerWin>`; any number may be
  open; each rescans its directory only when marked dirty.
- Minimize = skip rendering, restore/focus via taskbar (`SetWindowFocus`).
- `--screenshot`: hidden 1024×768 window, pre-opens one Explorer, renders 10
  frames, `glReadPixels` from the back buffer, vertical flip, force alpha,
  `stbi_write_png("screenshot.png")`, exit.

## 4. Verification loop (Phase 3)

Repeat ≥5×: clean-ish build → run `--screenshot` → open the PNG and inspect
against §1 → log discrepancies in CRITIQUE.md → fix. Check specifically:
bevel directions (raised vs sunken, window-frame vs button order), caption
height/color, taskbar alignment & clock, icon silhouettes, font legibility,
listview columns, status bar wells.

## 5. Finish (Phase 4)

Clean rebuild, final screenshot, README.md (macOS/Linux/Windows build+run),
STATUS.md (works / limitations / next steps, e.g. NeXTSTEP theme layer).
