#ifndef mesh_h
#define mesh_h
#pragma once

#include "camera.hpp"
#include "shader.hpp"
#include "skybox.hpp"
#include "renderable.hpp"

// System Headers
#include <assimp/importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standard Headers
#include <map>
#include <memory>
#include <vector>

// Define Namespace
namespace XK
{
    class Mesh : public Renderable
    {
    private:
    
        // Helper structs
        struct MatProps {
            aiColor3D ambientColor;
            aiColor3D diffuseColor;
            aiColor3D specularColor;
            float shininess;
        } matprops;
        
        RenderParams<TVertex> mRP;
        
    public:

        // Implement Default Constructor and Destructor
         Mesh() { glGenVertexArrays(1, &mVAO); }
        ~Mesh() { glDeleteVertexArrays(1, &mVAO); }

        // Implement Custom Constructors
        Mesh(kgr::container * container, Shader * shader, std::string const & filename);
        Mesh(
            std::vector<TVertex> const & vertices,
            std::vector<GLuint> const & indices,
            std::map<GLuint, std::string> const & textures
        );

        // Public Member Functions
        virtual void render() override;
        void draw(GLuint shader);
        virtual void translate(glm::vec3 coords) override;
        virtual void rotate(glm::quat rot) override;
        glm::mat4 & getModel();

    private:

        // Disable Copying and Assignment
        Mesh(Mesh const &) = delete;
        Mesh & operator=(Mesh const &) = delete;

        // Private Member Functions
        void createModelMatrix();
        void parse(std::string const & path, aiNode const * node, aiScene const * scene);
        void parse(std::string const & path, aiMesh const * mesh, aiScene const * scene);
        std::map<GLuint, std::string> process(std::string const & path,
                                              aiMaterial * material,
                                              aiTextureType type);

        // Private Member Containers
        std::vector<std::unique_ptr<Mesh>> mSubMeshes;
        std::vector<GLuint> mIndices;
        std::vector<TVertex> mVertices;
        std::map<GLuint, std::string> mTextures;

        // Private Member Variables
        Skybox * mSkybox = nullptr;

    };
};

#endif /* mesh_h */
