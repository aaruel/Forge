#ifndef XK_HPP
#define XK_HPP

#include <glad/glad.h>
#include <vector>
#include <memory>
#include "shadermanager.hpp"
#include "mesh.hpp"
#include "voxel.hpp"
#include "light.hpp"
#include "renderable.hpp"
#include "input.hpp"
#include "camera.hpp"
#include "skybox.hpp"

/**
 * This will define the state of the XK engine. Why is it called the XK engine? Because it sounds cool.
 * We will be using Kangaru as a dependency injection manager to pass around the XK service which will
 * be abstracted away from the outfacing API.
 */

namespace XK {
    struct XK {
        // Rendering Objects
        GLFWwindow& window;
        ShaderManager shaderManager;
        Pipeline renderPipeline;

        //Scene Objects
        std::unique_ptr<Camera> mainCamera;
        std::unique_ptr<Skybox> skybox;
        std::vector< std::reference_wrapper<Mesh> > meshes;
        std::vector< std::reference_wrapper<Voxel> > voxels;
        std::vector< std::reference_wrapper<Light> > lights;

        // You're not getting in here without arguments!
        XK() = delete;

        // Use a static initializer
        static XK Init(std::string windowName, int windowWidth, int windowHeight) {
            // Load GLFW and Create a Window
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
            auto window = glfwCreateWindow(
                windowWidth, windowHeight,
                windowName.c_str(),
                nullptr, nullptr
            );
            if (window == nullptr) throw "Failed to Create OpenGL Context";

            // Create Context and Load OpenGL Functions
            glfwMakeContextCurrent(window);
            gladLoadGL();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_STENCIL_TEST);
            fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));



            return XK(*window);
        }

    private:
        // Contructor initializes the window and the state
        XK(GLFWwindow& _window) : window(_window) {

        }

    };
}

#endif // XK_HPP
