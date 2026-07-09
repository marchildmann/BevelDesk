// Terminal drawer: a live shell docked below the taskbar (BevelDesk original).
#pragma once

struct AppState;
void ToggleDrawer(AppState& app);
// Pumps the shell every frame; renders the drawer strip when open.
// drawer_top = the y where the drawer begins (just below the taskbar).
void DrawTerminalDrawer(AppState& app, float drawer_top);
