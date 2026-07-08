# CRITIQUE — screenshot vs. Phase 1 research

## Iteration 1 (first successful render)

Overall: palette, teal desktop, navy caption, menu bar, header buttons, status
wells and clock all read correctly. Discrepancies found:

1. **Start button text clipped** — "Start" runs into the button's right bevel
   (button 54px is too narrow for ProggyClean + fake-bold overdraw).
   → widen to 64px.
2. **Stock ImGui scrollbar** in the file list: flat gray strip, no arrow
   buttons, no beveled thumb. Win95 scrollbars have 16px raised arrow buttons
   at both ends, a raised-bevel thumb, and a dithered (checkerboard) track.
   → implement a custom Theme95 vertical scrollbar.
3. **Caption title not bold** — Win95 captions used bold MS Sans Serif.
   → fake bold via 1px overdraw (same trick as the Start label).
4. **Address well** lacks the combo dropdown arrow button Win95's toolbar
   drive/path combo had at its right edge. → add (decorative) arrow button.
5. **Recycle Bin icon** slightly squat; opening ellipse merges into the body.
   → raise the rim, add a white highlight arc on the rim.
6. Screenshot demo shows a single window only — cannot verify the
   active/inactive caption distinction or multi-window support.
   → open a second Explorer (home dir) in --screenshot mode.

## Iteration 2 (start fixed, scrollbar, bold captions, 2 windows)

Verified: inactive caption is gray on the back window, navy + fake-bold on
the focused one; custom scrollbar shows arrow buttons and a beveled thumb;
Start label no longer clipped; address well has its combo arrow.
Discrepancy found:
1. **Type column ran under the scrollbar** ("File Folde…"): column layout
   used the full inner width, ignoring the 16px scrollbar.
   → columns now fit `inner_w - scrollbar`.

## Iteration 3 (column fit)

Verified: "File Folder" fully visible next to the scrollbar. New nits:
1. **"Size" header was left-aligned** — Win95 Details view uses
   LVCFMT_RIGHT for Size (header and values). → right-aligned the header.
2. No way to inspect Start menu / maximized chrome headlessly.
   → added `--demo start` / `--demo max` QA flags.

## Iteration 4 (demo states + 5x pixel zooms)

- `--demo start`: Start button correctly renders *pressed* while the menu is
  open; menu shows navy sidebar, separator groove, submenu arrows; menu sits
  flush above the taskbar at x=2. Flag logo panes read red/green/blue/yellow.
- `--demo max`: maximized window fills the work area above the taskbar and
  the maximize button switches to the restore glyph.
- 5x zoom of the chrome corner: window frame order (outer #DFDFDF, inner
  #FFFFFF) is correct and distinct from the button order — matches DrawEdge.
- Pixel probe (exact RGB): frame `223,223,223 / 255,255,255`, 4px border,
  caption starts exactly at +4 and is 18px; sunken list field
  `128,128,128 / near-black / white`. All match the research.
- Fix applied: DkShadow was #0A0A0A (98.css approximation); true Win95
  3DDkShadow is pure black → changed.

## Iteration 5 (interaction paths via --demo nav)

Programmatic navigation + minimize during the screenshot run:
- Window 1 navigated to /usr: caption + task button retitled "usr", path
  well shows /usr, listing correct (7 folders first, then files), status
  "9 object(s)". Scrollbar correctly absent when content fits, and columns
  expand to use its width.
- Window 2 minimized: gone from the desktop, still on the taskbar as a
  raised (unpressed) button; window 1 shows the pressed/lit task button.
- Symlinks (X11) list as "0KB File" — acceptable (documented in STATUS).

No remaining visual discrepancies at normal zoom. Final state matches the
Phase 1 research on: 16-color palette, bevel directions (all four variants),
18px caption, caption button anatomy, taskbar/Start/clock layout, Explorer
toolbar/listview/status structure, icon language.

## Iteration 7 (user feedback: interaction bugs found by play-testing)

Bugs no screenshot could catch, all in the custom scrollbar + list plumbing:
1. Thumb was never grabbable: the full-height track InvisibleButton was
   submitted first and claimed every press (ImGui blocks later items' hover
   while an active id is held). Fixed by submitting the thumb before the
   track; track ignores clicks inside the thumb rect.
2. Thumb dragged at half mouse speed: SetScrollY applies at the NEXT frame's
   Begin, so the same-frame GetScrollY readback restored the stale value and
   ate alternating deltas. Fixed with a scroll_pushed guard.
3. Scrollbar appeared too late on resize and under-scrolled: rows are 17px
   buttons but global ItemSpacing.y added 4px/row (real pitch 21px) while all
   math assumed 17. Fixed by zeroing ItemSpacing inside the list child — which
   is also the authentic Win95 row density.

## Iteration 6 (user feedback: font spacing)

User: ProggyClean's letter-spacing reads wrong — it's a monospaced font,
MS Sans Serif is proportional. Fixed by loading a proportional system UI
face (Tahoma 12px + Tahoma Bold on macOS; micross.ttf on Windows; DejaVu on
Linux). Captions, Start and task buttons now use real bold. Verified via 3x
zoom: natural proportional spacing, bold caption, all vertical centering
recomputed from the actual font size.

Also added in this pass (verified by screenshot):
- Shut Down dialog: full-screen 50% checkerboard dither fade (2x2 GL_REPEAT
  texture — one quad, not thousands of dots), modal click-blocker, dialog
  chrome variant (close-only caption, not resizable), radio buttons,
  Yes/No; "Restart the computer" resets the session, Esc/No/X cancel.
- Sortable column headers (Name/Size/Type, click toggles asc/desc, folders
  always first — headers render pressed while held, like COMCTL32).
- Large Icons view with 32px folder/document icons in a flowing grid +
  toolbar view-mode buttons; per-window view state.
