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
#include "gui.hpp"
#include "voxelscape.hpp"
#include "ubomanager.hpp"
#include "shadermanager.hpp"
#include "xk.hpp"

// System Headers
#include <glad/glad.h>
#include <kangaru/kangaru.hpp>
#include <glm/gtx/string_cast.hpp>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace XK;

int main() {
    XKState graphics = XKState::Init("Forge", 1024, 768);
    return 0;
}

int x__main() {
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
    glEnable(GL_STENCIL_TEST);
    //glEnable(GL_FRAMEBUFFER_SRGB);
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
    
    // Build shaders
    Shader defMesh;
    defMesh.attach("deferred/mesh.vert").attach("deferred/mesh.frag").link();
    Shader defVox;
    defVox.attach("deferred/voxel.vert").attach("deferred/voxel.frag").link();
    
    //Voxelscape<VoxelPlane> vs;
    
    // Instantiate camera
    //Camera * camera = Camera::getInstance();
    
    // Instantiate skybox
    kgr::container container;
    container.emplace<SkyboxService>("ocean/");
    Skybox& skybox = container.service<SkyboxService>();
    
    // Build gBuffer
    GBuffer gbuffer(mWindow);
    AmbientLight al;
    //PointLight sl;
    //DirectionalLight dl;
    EmissiveLight el;
    gbuffer.attach(&al).attach(&el);
    //sl.setPosition(glm::vec3(0.f, 0.4, -3.f));
    //sl.setDirection(glm::vec3(0.f, 0.f, 1.f));
    //dl.setDirection(glm::vec3(0.f, -1.f, 0.f));
    //dl.setPower(0.8f);
    
    // Voxel meshing
    Voxel voxel(&container, &defVox);
    voxel
        .addTexture("texture_diffuse", "snow/snow-base.png")
        .addTexture("normals", "snow/snow-normal.png")
        .addTexture("ambient", "snow/snow-ao.png")
        .addTexture("roughness", "snow/snow-roughness.png")
        .addTexture("heightmap", "snow/snow-height.png");
    
    Mesh mesh(&container, &defMesh, "helmet/DamagedHelmet.gltf");
    
    // Text console
    // Console console;
    
    // GUI
    //GUI::init(mWindow);
    
    Pipeline objects = {&mesh, &voxel};
    
    float mover = 0.0;
    glm::quat q = glm::angleAxis(glm::radians(90.f), glm::vec3(1.0, 0.0, 0.0));
    glm::quat r = glm::angleAxis(glm::radians(1.f), glm::vec3(0.0, 1.0, 0.0));
    mesh.rotate(q);
    bool slFollowCam = true;
    //Input::getInstance()->registerKeyEvent(GLFW_KEY_9, [&slFollowCam](){slFollowCam = !slFollowCam;});

    UBOManager<UBO::Float, UBO::Vector, UBO::Matrix> x("");
    
    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(mWindow, true);
        }

        // Background Fill Color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // rotate
        //mesh.rotate(r);

        // Step camera
        //camera->update();
        if (slFollowCam) {
            //sl.setPosition(camera->getPosition());
            //sl.setDirection(camera->getEye());
        }
        
        // gbuffer framebuffer rendering
        gbuffer.engage();
        
            // Draw skybox first so the actual map doesn't clip
            //sl.mRadiance.draw();
            Renderable::renderAll(&objects);
        
        gbuffer.disengage();
        
        gbuffer.createShadowMaps(&objects);

        // lighting pass
        gbuffer.runLighting();
        
        // Console rendering
        GUI::render();
        
        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
        mover += 0.01;
    }   glfwTerminate();
    GUI::destroy();
    return EXIT_SUCCESS;
}
