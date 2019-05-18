#ifndef mesh_h
#define mesh_h
#pragma once

#include "camera.hpp"
#include "shader.hpp"

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
    // Vertex Format
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    class Mesh
    {
    private:
    
        // Helper structs
        struct MatProps {
            aiColor3D ambientColor;
            aiColor3D diffuseColor;
            aiColor3D specularColor;
            float shininess;
        } matprops;
        
    public:

        // Implement Default Constructor and Destructor
         Mesh() { glGenVertexArrays(1, & mVertexArray); }
        ~Mesh() { glDeleteVertexArrays(1, & mVertexArray); }

        // Implement Custom Constructors
        Mesh(Shader * shader, std::string const & filename);
        Mesh(std::vector<Vertex> const & vertices,
             std::vector<GLuint> const & indices,
             std::map<GLuint, std::string> const & textures,
             MatProps _matprops);

        // Public Member Functions
        void draw();
        void draw(GLuint shader);
        void translate(glm::vec3 coords);
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
        std::vector<Vertex> mVertices;
        std::map<GLuint, std::string> mTextures;
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        Camera * mCamera;
        Shader * mShader;

        // Private Member Variables
        GLuint mVertexArray;
        GLuint mVertexBuffer;
        GLuint mElementBuffer;

    };
};

#endif /* mesh_h */
