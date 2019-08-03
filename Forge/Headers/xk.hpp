#ifndef XK_HPP
#define XK_HPP

// System Headers
#include <glad/glad.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

// Standard Headers
#include <vector>
#include <memory>
#include <fstream>
#include <map>
#include <functional>

// Local Headers
#include "shadermanager.hpp"
#include "mesh.hpp"
#include "voxel.hpp"
#include "light.hpp"
#include "renderable.hpp"
#include "input.hpp"
#include "camera.hpp"
#include "skybox.hpp"
#include "gbuffer.hpp"

// Config Defines
#define XK_CONFIG_ROOT PROJECT_SOURCE_DIR "/Config/"
#define XK_MAIN_CONFIG_LOCATION XK_CONFIG_ROOT "main.json"
#define XK_CONFIG_GET_STRING(config, location) std::string(config[location].GetString())
#define XK_JSON_GET_STRING(value) std::string(value.GetString())

namespace XK {
    struct Config {
        struct ShaderPair {
            std::string Vert;
            std::string Frag;
        };

        // Raw parsed configs
        rapidjson::Document mainConfig;
        std::unordered_map<std::string, rapidjson::Document> configs;

        // Meshes config
        std::vector<std::string> meshes;

        // Shaders config
        std::unordered_map<std::string, std::string> lights;
        std::unordered_map<std::string, ShaderPair> shaders;

        // Voxel config
        std::vector<std::string> materials;

        Config() {
            // Load main config
            std::ifstream jsonfilestream(XK_MAIN_CONFIG_LOCATION);
            rapidjson::IStreamWrapper jsonstream(jsonfilestream);
            mainConfig.ParseStream(jsonstream);

            for (auto iter = mainConfig.MemberBegin(); iter != mainConfig.MemberEnd(); ++iter) {
                std::string configName = XK_JSON_GET_STRING(iter->name);
                std::ifstream configStream(XK_CONFIG_ROOT + XK_JSON_GET_STRING(iter->value));
                rapidjson::IStreamWrapper iConfig(configStream);
                configs[configName].ParseStream(iConfig);
            }

            // Meshes parsing
            {
                auto& c = configs["MeshesConfig"];
                for (auto& mesh : c["Meshes"].GetArray()) {
                    meshes.push_back(XK_JSON_GET_STRING(mesh));
                }
            }

            // Shaders parsing
            {
                auto& c = configs["ShadersConfig"];
                auto& l = c["Lights"];
                for (auto iter = l.MemberBegin(); iter != l.MemberEnd(); ++iter) {
                    lights[XK_JSON_GET_STRING(iter->name)] = XK_JSON_GET_STRING(iter->value);
                }
                auto& s = c["Shaders"];
                for (auto iter = s.MemberBegin(); iter != s.MemberEnd(); ++iter) {
                    const auto& shader = iter->value.GetObject();
                    std::string vert = XK_JSON_GET_STRING(shader["Vert"]);
                    std::string frag = XK_JSON_GET_STRING(shader["Frag"]);
                    shaders[XK_JSON_GET_STRING(iter->name)] = ShaderPair{vert, frag};
                }
            }

            // Voxel parsing
            {
                auto& c = configs["VoxelConfig"];
                for (auto& material : c["Materials"].GetArray()) {
                    meshes.push_back(XK_JSON_GET_STRING(material));
                }
            }
        }
    };

    /**
     * This will define the state of the XK engine. Why is it called the XK engine? Because it sounds cool.
     * We will be using Kangaru as a dependency injection manager to pass around the XK service which will
     * be abstracted away from the outfacing API.
     */
    struct XKState {
        // Rendering Objects
        GLFWwindow* window;
        ShaderManager shaderManager;
        Pipeline renderPipeline;

        //Scene Objects
        std::unique_ptr<Input> input;
        std::unique_ptr<GBuffer> gBuffer;
        std::unique_ptr<Camera> mainCamera;
        std::unique_ptr<Skybox> skybox;
        std::unordered_map<std::string, Shader> shaders;
        std::vector<Mesh> meshes;
        std::vector<Voxel> voxels;
        std::vector<Light> lights;

        // Configuration Data
        Config config;

        // You're not getting in here without arguments!
        XKState() = delete;

        // Use a static initializer
        static XKState Init(std::string windowName, int windowWidth, int windowHeight) {
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

            return XKState(window);
        }

        void renderloop(std::function<void()> hook = [](){}) {
            while (glfwWindowShouldClose(window) == false) {
                if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                    glfwSetWindowShouldClose(window, true);
                }

                // Background Fill Color
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Run a payload with the exposed object state (probably should send the instance with _this_?)
                hook();

                // Flip Buffers and Draw
                glfwSwapBuffers(window);
                glfwPollEvents();
            }
        }

        ~XKState() {
            glfwTerminate();
        }

    private:
        // Contructor initializes the window and the state
        XKState(GLFWwindow* _window, Config _config = Config()) :
            window(_window),
            input(std::make_unique<Input>(_window)),
            gBuffer(std::make_unique<GBuffer>(_window)),
            mainCamera(std::make_unique<Camera>(input.get())),
            config(std::move(_config))
        {
            // Instantiate skybox, no actual handler for this yet...
            kgr::container container;
            container.emplace<SkyboxService>("ocean/");
            Skybox& skybox = container.service<SkyboxService>();

            for(auto& [name, pair] : config.shaders) {
                shaders[name].attach(pair.Vert).attach(pair.Frag).link();
            }

            for (auto& mesh : config.meshes) {
                // this applies to the mesh shader
                const std::string name = "Mesh";
                meshes.push_back(Mesh(&container, &shaders[name], mesh));
            }

            std::cout << "XK Engine Loaded" << std::endl;
        }

    };
}

#endif // XK_HPP
