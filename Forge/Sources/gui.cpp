//
//  gui.cpp
//  Forge
//
//  Created by Aaron Ruel on 4/20/19.
//

#include "gui.hpp"

// System Headers
#include <cstring>
#include <cstdarg>
#include <glad/glad.h>
#include <fstream>
#include <map>
#include <kangaru/kangaru.hpp>

using namespace std;

// Nuklear declarations
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear.h>
#include "nuklear_glfw.hpp"

// Local Headers
#include "input.hpp"
#include "console.hpp"
#include "shader.hpp"
#include "utils.hpp"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

static struct nk_context * _ctx = nullptr;
static struct nk_font_atlas * atlas;
static struct nk_colorf bg;
GLFWwindow * _window;
Input * inputInst;
// Input buffer
Buffer consoleBuffer = std::make_unique<char[]>(BufferSize);
// DI
kgr::container container;
// Variable sized lua console
Console<> &console = container.service<ConsoleService<>>();

void XK::GUI::init(XKState& xk) {
    inputInst = xk.input.get();
    _window = xk.window;
    _ctx = nk_glfw3_init(
        _window,
        NK_GLFW3_INSTALL_CALLBACKS
    );
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();
    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    inputInst->registerKeyEvent(GLFW_KEY_0, [](){
        inputInst->toggleMouseCapture();
    });
}

void input_console() {
    if (nk_begin(
        _ctx,
        "Console",
        nk_rect(50, 50, 430, 250),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE
    )) {
        const Buffer & luaBuffer = console.getConsoleBuffer();
        nk_layout_row_dynamic(_ctx, 160, 1);
        nk_edit_string_zero_terminated(
            _ctx,
            NK_EDIT_EDITOR,
            luaBuffer.get(), BufferSize,
            nk_filter_ascii
        );
        nk_layout_row_dynamic(_ctx, 30, 2);
        nk_edit_string_zero_terminated(
            _ctx,
            NK_EDIT_BOX,
            consoleBuffer.get(), BufferSize,
            nk_filter_ascii
        );
        if (nk_button_label(_ctx, "Submit")) {
            // send to the executing function
            console.executeLua(consoleBuffer);
            
            // clear the buffer
            memset(consoleBuffer.get(), 0, 255);
        }
    }
    nk_end(_ctx);
}

void shader_editor() {
    constexpr size_t buffersize = 16384;
    static Buffer buffer = make_buffer(buffersize);
    static std::vector<std::string> paths;
    static std::vector<const char*> oldPaths;
    static std::vector<XK::Shader*> programMap;
    static std::string file = "";
    static int selector = 0;
    static bool populate = true;
    static bool firstLoad = true;
    if (nk_begin(
        _ctx, "Shader Editor",
        nk_rect(50, 400, 700, 500),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE
    )) {
        // selector
        nk_layout_row_dynamic(_ctx, 30, 3);
        if (populate || nk_button_label(_ctx, "Refresh")) {
            paths.clear();
            oldPaths.clear();
            programMap.clear();
            // populate with each path
            for (XK::Shader * shader : XK::Shader::getAllShaders())
            for (std::string path : shader->getPaths()) {
                paths.push_back(path);
                programMap.push_back(shader);
            }
            // convert to old char**
            for (const auto& path : paths) {
                oldPaths.push_back(path.c_str());
            }
            populate = false;
        }
        nk_label(_ctx, "Active Shader:", NK_TEXT_ALIGN_RIGHT);
        int previousSelector = selector;
        selector = nk_combo(_ctx, oldPaths.data(), oldPaths.size(), selector, 30, nk_vec2(200, 200));
        if (firstLoad || previousSelector != selector) {
            // loading
            std::string path = PROJECT_SOURCE_DIR "/Shaders/" + paths[selector];
            std::ifstream fd(path);
            if (file.size()) memset(buffer.get(), 0, file.size()-1);
            file = std::string(
                std::istreambuf_iterator<char>(fd),
                std::istreambuf_iterator<char>()
            );
            file.copy(buffer.get(), file.size());
            firstLoad = false;
        }
        // editor
        nk_layout_row_dynamic(_ctx, 380, 1);
        nk_edit_string_zero_terminated(
            _ctx,
            NK_EDIT_BOX,
            buffer.get(),
            buffersize,
            nk_filter_ascii
        );
        // Reload shader
        nk_layout_row_dynamic(_ctx, 30, 1);
        if (nk_button_label(_ctx, "Reload")) {
            std::string path = PROJECT_SOURCE_DIR "/Shaders/" + paths[selector];
            ofstream newFile;
            newFile.open(path);
            newFile << buffer.get();
            newFile.close();
            programMap[selector]->reload();
        }
    }
    nk_end(_ctx);
}

void close_windows() {
    int width, height;
    glfwGetWindowSize(_window, &width, &height);
    if (nk_begin(_ctx, "Close", nk_rect(width-100, 40, 50, 70), NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(_ctx, 30, 1);
        if (nk_button_label(_ctx, "X")) inputInst->toggleMouseCapture();
    }
    nk_end(_ctx);
}

void XK::GUI::render() {
    // ignore gui unless toggled
    if (inputInst->mouseIsCaptured()) return;
    nk_glfw3_new_frame();
    input_console();
    shader_editor();
    close_windows();
    nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
}

void XK::GUI::destroy() {
    nk_glfw3_shutdown();
}
