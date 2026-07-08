// MS-DOS Prompt — a real shell (zsh) on a pty inside Win95 chrome.
#pragma once

struct AppState;
// Spawns a shell and opens a prompt window (no-op if the pty can't spawn).
void OpenDosPrompt(AppState& app);
void DrawDosPrompts(AppState& app);
