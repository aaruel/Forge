// Preprocessor Directives
#define STB_IMAGE_IMPLEMENTATION

// Local Headers
#include "mesh.hpp"

// System Headers
#include <stb_image.h>
#include <glm/gtx/string_cast.hpp>

// Define Namespace
namespace XK
{
    Mesh::Mesh(kgr::container * container, Shader * shader, std::string const & filename) : Mesh()
    {
        // Dependecy inject the camera and shader
        mContainer = container;
        mShader = shader;
        mCamera = Camera::getInstance();
        mSkybox = &mContainer->service<SkyboxService>();
        // Load a Model from File
        Assimp::Importer loader;
        aiScene const * scene = loader.ReadFile(
            PROJECT_SOURCE_DIR "/Models/" + filename,
            aiProcessPreset_TargetRealtime_MaxQuality |
            aiProcess_CalcTangentSpace                |
            aiProcess_OptimizeGraph                   |
            aiProcess_FlipUVs);

        // Walk the Tree of Scene Nodes
        auto index = filename.find_last_of("/");
        if (!scene) fprintf(stderr, "%s\n", loader.GetErrorString());
        else parse(filename.substr(0, index), scene->mRootNode, scene);
    }

    Mesh::Mesh(std::vector<TVertex> const & vertices,
               std::vector<GLuint> const & indices,
               std::map<GLuint, std::string> const & textures)
                    : mIndices(indices)
                    , mVertices(vertices)
                    , mTextures(textures)
    {
        mRP.vertices = mVertices;
        mRP.indices = mIndices;
        uploadToGPU(&mRP);
    }
    
    void Mesh::translate(glm::vec3 coords) {
        Renderable::translate(coords);
        for (auto &i : mSubMeshes)
            i->translate(coords);
    }
    
    void Mesh::rotate(glm::quat rot) {
        Renderable::rotate(rot);
        for (auto &i : mSubMeshes)
            i->rotate(rot);
    }
    
    glm::mat4 & Mesh::getModel() {
        return mModel;
    }

    void Mesh::render() {
        // Make sure the drawing shader is correct
        mShader->activate();
        unsigned shader = mShader->get();
        // Passes view/projection to the mesh shader
        mCamera->render(shader);
        // Pass skybox for reflection
        glActiveTexture(GL_TEXTURE31);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mSkybox->getTextureId());
        glUniform1i(glGetUniformLocation(shader, "cubemap"), 31);
        glActiveTexture(GL_TEXTURE30);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mRadiance.getTextureId());
        glUniform1i(glGetUniformLocation(shader, "radiance"), 30);
        glActiveTexture(GL_TEXTURE29);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mIrradiance.getTextureId());
        glUniform1i(glGetUniformLocation(shader, "irradiance"), 29);
        // Pass to recursive function
        draw(shader);
    }

    void Mesh::draw(GLuint shader)
    {
        unsigned int unit = 0, diffuse = 0, specular = 0, normals = 0, ambient = 0, emissive = 0, unknown = 0;
        for (auto &i : mSubMeshes)
            i->draw(shader);
        for (auto &i : mTextures)
        {   // Set Correct Uniform Names Using Texture Type (Omit ID for 0th Texture)
            std::string uniform = i.second;
                 if (i.second == "diffuse")  uniform += (diffuse++  > 0) ? std::to_string(diffuse)  : "";
            else if (i.second == "specular") uniform += (specular++ > 0) ? std::to_string(specular) : "";
            else if (i.second == "normals")  uniform += (normals++  > 0) ? std::to_string(normals)  : "";
            else if (i.second == "ambient")  uniform += (ambient++  > 0) ? std::to_string(ambient)  : "";
            else if (i.second == "emissive") uniform += (emissive++ > 0) ? std::to_string(emissive) : "";
            else if (i.second == "unknown")  uniform += (unknown++  > 0) ? std::to_string(unknown)  : "";
            //printf("%s %i\n", i.second.c_str(), i.first);
            // Bind Correct Textures and Vertex Array Before Drawing
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, i.first);
            glUniform1i(glGetUniformLocation(shader, uniform.c_str()), unit++);
        }
        // Apply shader uniforms
        GLint Umodel = glGetUniformLocation(shader, "model");
        glUniformMatrix4fv(Umodel, 1, GL_FALSE, glm::value_ptr(mModel));
        if (mIndices.size() == 0) return;
        // Draw vertices
        glBindVertexArray(mRP.vao);
        // Runs shaders :)
        glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, nullptr);
    }

    void Mesh::parse(std::string const & path, aiNode const * node, aiScene const * scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
            parse(path, scene->mMeshes[node->mMeshes[i]], scene);
        for (unsigned int i = 0; i < node->mNumChildren; i++)
            parse(path, node->mChildren[i], scene);
    }

    void Mesh::parse(std::string const & path, aiMesh const * mesh, aiScene const * scene)
    {
        // Create Vertex Data from Mesh Node
        std::vector<TVertex> vertices; TVertex vertex;
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        if (mesh->mTextureCoords[0])
            std::get<uv>(vertex) = glm::vec2(
                mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y
            );
            std::get<position>(vertex) = glm::vec3(
                mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z
            );
            std::get<normal>(vertex) = glm::vec3(
                mesh->mNormals[i].x,  mesh->mNormals[i].y,  mesh->mNormals[i].z
            );
            std::get<tangent>(vertex) = glm::vec3(
                mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z
            );
            std::get<bitangent>(vertex) = glm::vec3(
                mesh->mBitangents[i].x,mesh->mBitangents[i].y,mesh->mBitangents[i].z
            );
            vertices.push_back(vertex);
        }

        // Create Mesh Indices for Indexed Drawing
        std::vector<GLuint> indices;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
            indices.push_back(mesh->mFaces[i].mIndices[j]);

        // Load material properties
        aiMaterial * mat = scene->mMaterials[mesh->mMaterialIndex];

        // Load Mesh Textures into VRAM
        std::map<GLuint, std::string> textures;
        auto diffuse  = process(path, mat, aiTextureType_DIFFUSE);
        auto specular = process(path, mat, aiTextureType_SPECULAR);
        auto normals  = process(path, mat, aiTextureType_NORMALS);
        auto ambient  = process(path, mat, aiTextureType_AMBIENT);
        auto emissive = process(path, mat, aiTextureType_EMISSIVE);
        auto unknown  = process(path, mat, aiTextureType_UNKNOWN);
        textures.insert(diffuse.begin(), diffuse.end());
        textures.insert(specular.begin(), specular.end());
        textures.insert(normals.begin(), normals.end());
        textures.insert(ambient.begin(), ambient.end());
        textures.insert(emissive.begin(), emissive.end());
        textures.insert(unknown.begin(), unknown.end());

        // Create New Mesh Node
        mSubMeshes.push_back(std::unique_ptr<Mesh>(new Mesh(vertices, indices, textures)));
    }

    std::map<GLuint, std::string> Mesh::process(std::string const & path,
                                                aiMaterial * material,
                                                aiTextureType type)
    {
        std::map<GLuint, std::string> textures;
        for(unsigned int i = 0; i < material->GetTextureCount(type); i++)
        {
            // Define Some Local Variables
            GLenum format;
            GLuint texture;
            std::string mode;

            // Load the Texture Image from File
            aiString str; material->GetTexture(type, i, & str);
            std::string filename = str.C_Str(); int width, height, channels;
            filename = PROJECT_SOURCE_DIR "/Models/" + path + "/" + filename;
            unsigned char * image = stbi_load(filename.c_str(), & width, & height, & channels, 0);
            if (!image) fprintf(stderr, "%s %s\n", "Failed to Load Texture", filename.c_str());

            // Set the Correct Channel Format
            switch (channels) {
                case 1 : format = GL_ALPHA;     break;
                case 2 : format = GL_LUMINANCE; break;
                case 3 : format = GL_RGB;       break;
                case 4 : format = GL_RGBA;      break;
            }

            // Bind Texture and Set Filtering Levels
            glGenTextures(1, & texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, format,
                         width, height, 0, format, GL_UNSIGNED_BYTE, image);
            glGenerateMipmap(GL_TEXTURE_2D);

            // Release Image Pointer and Store the Texture
            stbi_image_free(image);
                 if (type == aiTextureType_DIFFUSE)  mode = "diffuse";
            else if (type == aiTextureType_SPECULAR) mode = "specular";
            else if (type == aiTextureType_NORMALS)  mode = "normals";
            else if (type == aiTextureType_AMBIENT)  mode = "ambient";
            else if (type == aiTextureType_EMISSIVE) mode = "emissive";
            else if (type == aiTextureType_UNKNOWN)  mode = "unknown";
            textures.insert(std::make_pair(texture, mode));
        }   return textures;
    }
};
