// Local Headers
#include "glitter.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "input.hpp"
#include "skybox.hpp"
#include "voxel.hpp"
#include "console.hpp"

// System Headers
#include <glad/glad.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>

using namespace XK;

int main() {
    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);
    GlobalSingleton::getInstance()->setWindow(mWindow);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    glEnable(GL_DEPTH_TEST);
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
    
    // Build shader
    Shader shader;
    shader.attach("phong.vert").attach("phong.frag").link();
    
    Shader voxelShader;
    voxelShader.attach("voxel.vert").attach("voxel.frag").link();
    
    // Instantiate camera
    Camera * camera = Camera::getInstance();
    
    // Instantiate skybox
    Skybox skybox("space/");
    
    // Voxel meshing
    Voxel voxel(&voxelShader);
    
    // Text console
    Console console;
    
    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Step camera
        camera->update();
        
        // Draw skybox first so the actual map doesn't clip
        skybox.draw();
        
        // Voxel rendering
        voxel.render();
        
        // Console rendering
        console.render();
        
        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    
    return EXIT_SUCCESS;
}
