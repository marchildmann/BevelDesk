#include "theme95/style.h"
#include "theme95/palette.h"

namespace t95 {

void ApplyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = s.ChildRounding = s.FrameRounding = 0.0f;
    s.PopupRounding = s.ScrollbarRounding = s.GrabRounding = s.TabRounding = 0.0f;
    s.WindowBorderSize = s.ChildBorderSize = s.PopupBorderSize = s.FrameBorderSize = 0.0f;
    s.ScrollbarSize = 16.0f;
    s.AntiAliasedLines = false;
    s.AntiAliasedLinesUseTex = false;
    s.AntiAliasedFill = false;
    s.ItemSpacing = ImVec2(4, 4);

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(TEXT);
    c[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(FACE);
    c[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ScrollbarBg] = ImGui::ColorConvertU32ToFloat4(IM_COL32(222, 222, 222, 255));
    c[ImGuiCol_ScrollbarGrab] = ImGui::ColorConvertU32ToFloat4(FACE);
    c[ImGuiCol_ScrollbarGrabHovered] = ImGui::ColorConvertU32ToFloat4(FACE);
    c[ImGuiCol_ScrollbarGrabActive] = ImGui::ColorConvertU32ToFloat4(FACE);
    c[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(WINBG);
    c[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(SEL);
    c[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(SEL);
    c[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(SEL);
}

} // namespace t95
