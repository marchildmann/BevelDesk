#include "app/app.h"

namespace fs = std::filesystem;

static std::string TitleFor(const fs::path& p) {
    std::string name = p.filename().string();
    if (name.empty()) name = p.string();          // "/" at the root
    if (name.empty()) name = "My Computer";
    return name;
}

void ExplorerWin::Navigate(const fs::path& p) {
    path = p;
    title = TitleFor(p);
    selected = -1;
    scroll_y = 0;
    scroll_set = true;
    dirty = true;
}

void AppState::OpenExplorer(const fs::path& p) {
    ExplorerWin w;
    w.id = next_id++;
    w.imgui_id = "##explorer" + std::to_string(w.id);
    int n = (int)wins.size();
    w.def_pos = ImVec2(90.0f + 26.0f * n, 70.0f + 26.0f * n);
    w.def_size = ImVec2(460, 340);
    w.request_focus = kRefocusFrames;
    w.Navigate(p);
    wins.push_back(std::move(w));
    active_win_id = wins.back().id;
}
