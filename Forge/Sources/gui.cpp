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
#include "input.hpp"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

static struct nk_context * _ctx = nullptr;
static struct nk_font_atlas * atlas;
static struct nk_colorf bg;
GLFWwindow * _window;
Input * inputInst;
char consoleBuffer[256];

void XK::GUI::init(GLFWwindow * window) {
    inputInst = Input::getInstance();
    _window = window;
    _ctx = nk_glfw3_init(
        window,
        NK_GLFW3_INSTALL_CALLBACKS
    );
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();
    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    inputInst->registerKeyEvent(GLFW_KEY_0, [](){
        inputInst->toggleMouseCapture();
    });
}

void input_console(void (*exec)(char*)) {
    if (nk_begin(
        _ctx,
        "Console",
        nk_rect(50, 50, 230, 250),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE
    )) {
        nk_layout_row_dynamic(_ctx, 160, 1);
        nk_edit_string_zero_terminated(
            _ctx,
            NK_EDIT_EDITOR,
            consoleBuffer, sizeof(consoleBuffer),
            nk_filter_ascii
        );
        nk_layout_row_dynamic(_ctx, 30, 2);
        nk_edit_string_zero_terminated(
            _ctx,
            NK_EDIT_BOX,
            consoleBuffer, sizeof(consoleBuffer),
            nk_filter_ascii
        );
        if (nk_button_label(_ctx, "Submit")) {
            // send to the executing function
            if (exec) exec(consoleBuffer);
            
            // clear the buffer
            memset(consoleBuffer, 0, 255);
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
    input_console(nullptr);
    close_windows();
    nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
}

void XK::GUI::destroy() {
    nk_glfw3_shutdown();
}
