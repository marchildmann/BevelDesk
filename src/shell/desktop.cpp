#include "shell/desktop.h"
#include "app/app.h"
#include "theme95/theme95.h"
#include "theme95/icons95.h"
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;
using namespace t95;

static fs::path HomePath() {
    const char* h = std::getenv("HOME");
#ifdef _WIN32
    if (!h) h = std::getenv("USERPROFILE");
#endif
    return h ? fs::path(h) : fs::path("/");
}

static fs::path TrashPath() {
    fs::path t = HomePath() / ".Trash";
    std::error_code ec;
    if (fs::is_directory(t, ec)) return t;
    return HomePath();
}

void DrawDesktop(AppState& app) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(DESKTOP));
    ImGui::Begin("##desktop", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    struct DeskIcon {
        const char* label;
        void (*icon)(ImDrawList*, ImVec2);
    };
    static const DeskIcon icons[] = {
        { "My Computer", icons95::MyComputer32 },
        { "Recycle Bin", icons95::RecycleBin32 },
    };

    bool any_icon_hovered = false;
    for (int i = 0; i < 2; ++i) {
        ImVec2 cell(vp->Pos.x + 14, vp->Pos.y + 16 + i * 78.0f);
        ImVec2 cell_sz(72, 56);
        ImGui::SetCursorScreenPos(cell);
        ImGui::PushID(i);
        ImGui::InvisibleButton("##deskicon", cell_sz);
        ImGui::PopID();
        bool hovered = ImGui::IsItemHovered();
        any_icon_hovered |= hovered;
        if (hovered && ImGui::IsMouseClicked(0)) app.desktop_selected = i;
        if (hovered && ImGui::IsMouseDoubleClicked(0)) {
            if (i == 0) app.OpenExplorer("/");
            else        app.OpenExplorer(TrashPath());
        }

        icons[i].icon(dl, ImVec2(cell.x + (cell_sz.x - 32) * 0.5f, cell.y));
        ImVec2 ts = ImGui::CalcTextSize(icons[i].label);
        ImVec2 tp(cell.x + (cell_sz.x - ts.x) * 0.5f, cell.y + 37);
        if (app.desktop_selected == i)
            dl->AddRectFilled(ImVec2(tp.x - 2, tp.y - 1), ImVec2(tp.x + ts.x + 2, tp.y + ts.y + 1), SEL);
        dl->AddText(tp, HILIGHT, icons[i].label);
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !any_icon_hovered)
        app.desktop_selected = -1;

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}
