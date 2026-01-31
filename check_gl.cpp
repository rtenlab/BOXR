#include <X11/Xlib.h>
#include <GL/glx.h>
#include <iostream>
#include <vector>

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092

int main() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        std::cerr << "Cannot connect to X server" << std::endl;
        return 1;
    }

    int screen = DefaultScreen(dpy);
    std::cout << "Connected to X server. Default screen: " << screen << std::endl;

    // Attributes from extended_window.hpp
    std::vector<int> visual_attribs = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };

    int fbcount = 0;
    GLXFBConfig* fbc = glXChooseFBConfig(dpy, screen, visual_attribs.data(), &fbcount);
    if (fbc) {
        std::cout << "Success! Found " << fbcount << " matching framebuffer configs." << std::endl;
        XFree(fbc);
    } else {
        std::cout << "Failed to retrieve a framebuffer config with full attributes." << std::endl;
        
        // Diagnostic: Try removing attributes one by one/groups to see what fails
        struct AttribGroup {
            std::string name;
            std::vector<int> attribs;
        };

        std::vector<AttribGroup> tests = {
            {"Minimal (RGBA)", {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, None}},
            {"RGBA + DoubleBuffer", {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DOUBLEBUFFER, True, None}},
            {"RGBA + Depth 24", {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DEPTH_SIZE, 24, None}},
            {"RGBA + Stencil 8", {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_STENCIL_SIZE, 8, None}},
            {"RGBA + Alpha 8", {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_ALPHA_SIZE, 8, None}},
             {"RGBA + Alpha 8 + Stencil 8", {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_ALPHA_SIZE, 8, GLX_STENCIL_SIZE, 8, None}},
        };

        for (const auto& test : tests) {
            int count = 0;
            GLXFBConfig* res = glXChooseFBConfig(dpy, screen, test.attribs.data(), &count);
            if (res) {
                std::cout << "  [OK] " << test.name << " (Found " << count << ")" << std::endl;
                XFree(res);
            } else {
                std::cout << "  [FAIL] " << test.name << std::endl;
            }
        }
    }

    XCloseDisplay(dpy);
    return 0;
}
