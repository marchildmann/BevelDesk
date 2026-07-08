// Platform glue: fonts, GL textures, framebuffer screenshots.
#pragma once
#include "imgui.h"

struct GLFWwindow;

// Load the closest proportional system UI face (Tahoma / Microsoft Sans
// Serif / DejaVu, per OS) at 12px plus its bold variant (t95::FontBold);
// falls back to ImGui's default font. `density` = monitor content scale
// (2.0 on Retina) so glyphs rasterize sharp.
void LoadFonts(ImGuiIO& io, float density);

// 2x2 black/transparent checkerboard, GL_REPEAT — the Shut Down screen fade.
ImTextureID CreateCheckerTexture();

// Read the back buffer, flip, force opaque alpha, write a PNG.
bool SaveScreenshot(GLFWwindow* window, const char* path);
