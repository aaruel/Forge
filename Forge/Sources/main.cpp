// Local Headers
#include "glitter.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "input.hpp"
#include "skybox.hpp"
#include "voxel.hpp"
#include "console.hpp"
#include "gbuffer.hpp"
#include "light.hpp"

// System Headers
#include <glad/glad.h>
#include "gui.hpp"

// Standard Headers
#include <cstdio>
#include <cstdlib>

using namespace XK;

int main() {
    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
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
    //glEnable(GL_FRAMEBUFFER_SRGB);
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
    
    // Build shader
    Shader shader;
    shader.attach("main.vert").attach("main.frag").link();
    
    Shader voxelShader;
    voxelShader.attach("voxel2.vert").attach("voxel2.frag").link();
    
    // Instantiate camera
    Camera * camera = Camera::getInstance();
    
    // Instantiate skybox
    Skybox skybox("space/");
    
    // Build gBuffer
    GBuffer gbuffer(mWindow);
    DirectionalLight dl;
    PointLight pl;
    AmbientLight al;
    gbuffer.attach(&al).attach(&dl).attach(&pl);
    
    // Voxel meshing
    Voxel voxel(gbuffer.getShader());
    voxel
        .addTexture("texture_diffuse", "snow/snow-base.png")
        .addTexture("SnowNormal", "snow/snow-normal.png")
        .addTexture("SSAO", "snow/snow-ao.png");
    
    Mesh mesh(gbuffer.getShader(), "sponza/sponza.obj");
    
    // Text console
    // Console console;
    
    // GUI
    GUI::init(mWindow);
    
    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw skybox first so the actual map doesn't clip
        //skybox.draw();
        
        // Step camera
        camera->update();
        dl.setPosition(camera->getPosition());
        pl.setPosition(camera->getPosition());
        
        // gbuffer framebuffer rendering
        gbuffer.engage();
        
            // Voxel rendering
            //voxel.render();
            mesh.draw();
        
        gbuffer.disengage();
        
        // lighting pass
        gbuffer.runLighting();
        
        // Console rendering
        GUI::render();
        
        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    XK::GUI::destroy();
    return EXIT_SUCCESS;
}
