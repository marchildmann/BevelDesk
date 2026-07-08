// Explorer windows — real-filesystem folder browsing.
#pragma once
#include <string>

struct AppState;
struct ExplorerWin;

void DrawExplorers(AppState& app);
// Re-sort w.entries by w.sort_col / w.sort_asc (folders always first).
void SortEntries(ExplorerWin& w);
// "1,234KB" (rounded up), Win95 Details style.
std::string FormatKB(unsigned long long bytes);
