#include "platform/platform.h"
#include "platform/gl.h"
#include "theme95/widgets.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include "stb_image_write.h"

static bool FileExists(const char* path) {
    if (FILE* f = std::fopen(path, "rb")) { std::fclose(f); return true; }
    return false;
}

void LoadFonts(ImGuiIO& io, float density) {
    static const char* regular[] = {
#if defined(__APPLE__)
        "/System/Library/Fonts/Supplemental/Tahoma.ttf",
        "/System/Library/Fonts/Supplemental/Verdana.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
#elif defined(_WIN32)
        "C:\\Windows\\Fonts\\micross.ttf",   // Microsoft Sans Serif — the real deal
        "C:\\Windows\\Fonts\\tahoma.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
#endif
    };
    static const char* bold[] = {
#if defined(__APPLE__)
        "/System/Library/Fonts/Supplemental/Tahoma Bold.ttf",
        "/System/Library/Fonts/Supplemental/Verdana Bold.ttf",
        "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
#elif defined(_WIN32)
        "C:\\Windows\\Fonts\\tahomabd.ttf",
        "C:\\Windows\\Fonts\\arialbd.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
#endif
    };
    const float SIZE = 12.0f;               // MS Sans Serif 8pt ~= 11-12px cell
    ImFontConfig cfg;
    cfg.OversampleH = 2;
    cfg.OversampleV = 1;
    cfg.PixelSnapH = true;
    cfg.RasterizerDensity = density > 1.0f ? density : 1.0f;
    static const char* mono[] = {
#if defined(__APPLE__)
        "/System/Library/Fonts/Monaco.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
#elif defined(_WIN32)
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
#endif
    };
    ImFont* base = nullptr;
    for (const char* p : regular)
        if (FileExists(p) && (base = io.Fonts->AddFontFromFileTTF(p, SIZE, &cfg))) break;
    if (!base) io.Fonts->AddFontDefault();
    for (const char* p : bold)
        if (FileExists(p) && (t95::FontBold = io.Fonts->AddFontFromFileTTF(p, SIZE, &cfg))) break;
    for (const char* p : mono)
        if (FileExists(p) && (t95::FontMono = io.Fonts->AddFontFromFileTTF(p, SIZE, &cfg))) break;
}

ImTextureID CreateCheckerTexture() {
    static const unsigned char px[16] = {
        0, 0, 0, 255,  0, 0, 0, 0,
        0, 0, 0, 0,    0, 0, 0, 255,
    };
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (ImTextureID)(intptr_t)tex;
}

bool SaveScreenshot(GLFWwindow* window, const char* path) {
    int fw = 0, fh = 0;
    glfwGetFramebufferSize(window, &fw, &fh);
    if (fw <= 0 || fh <= 0) return false;
    std::vector<unsigned char> px((size_t)fw * fh * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, fw, fh, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    // flip vertically + force opaque alpha
    std::vector<unsigned char> out((size_t)fw * fh * 4);
    for (int yrow = 0; yrow < fh; ++yrow) {
        const unsigned char* src = px.data() + (size_t)(fh - 1 - yrow) * fw * 4;
        unsigned char* dst = out.data() + (size_t)yrow * fw * 4;
        std::memcpy(dst, src, (size_t)fw * 4);
        for (int x = 0; x < fw; ++x) dst[x * 4 + 3] = 255;
    }
    int ok = stbi_write_png(path, fw, fh, 4, out.data(), fw * 4);
    if (ok) std::printf("Saved %s (%dx%d)\n", path, fw, fh);
    else    std::fprintf(stderr, "Failed to write %s\n", path);
    return ok != 0;
}
