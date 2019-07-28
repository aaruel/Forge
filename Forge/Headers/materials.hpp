#ifndef MATERIALS_HPP
#define MATERIALS_HPP

// Local Headers
#include "shader.hpp"

// System Headers
#include <stb_image.h>
#include <string>
#include <fstream>
#include <vector>

// Third Party
#include <glad/glad.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

namespace XK {
    template<GLenum TexType>
    class Texture {
    private:
        constexpr GLenum getFormat(GLint nrChannels) {
            // Set the Correct Channel Format
            switch (nrChannels) {
                case 1 : return GL_ALPHA;
                case 2 : return GL_LUMINANCE;
                case 3 : return GL_RGB;
                case 4 : return GL_RGBA;
                default: return GL_INVALID_ENUM;
            }
        }

        // Paths should be relative to project root
        void loadTexture(const std::string& path) {
            const std::string texture = PROJECT_SOURCE_DIR "/" + path;
            GLint width, height, nrChannels;
            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
            GLenum format = getFormat(nrChannels);
            GLenum imageFormat = GL_UNSIGNED_BYTE;
            // Load texture
            glGenTextures(1, &mTextureId);
            glBindTexture(TexType, mTextureId);
            glTexStorage2D(TexType, 1, format, width, height);
            glTexSubImage2D(TexType, 0, 0, 0, width, height, format, imageFormat, data);
            glTexParameteri(TexType, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(TexType, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(TexType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(TexType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }

    public:
        template<typename T>
        Texture(T&& bindingName, T&& path) :
            mBindingName(std::forward(bindingName)),
            mPath(std::forward(path))
        {
            loadTexture(mPath);
        }

        void upload(Shader& shader, GLuint unit) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(TexType, mTextureId);
            shader.bind(mBindingName, static_cast<int>(unit));
        }

    private:
        std::string mBindingName;
        std::string mPath;
        GLuint mTextureId;
    };

    /**
     * A class for models that don't define an order for texture loading, like for voxels
     */
    class Material {
    private:
        enum mEnumTypes {
            diffuse = 0,
            specular,
            normal,
            ambientOcclusion,
            emissive
        };

    public:
        template<typename T>
        Material(const T&& path) : mPath(std::forward(path)) {
            mPath = PROJECT_SOURCE_DIR "/" + mPath;
            std::ifstream jsonfilestream(mPath);
            rapidjson::IStreamWrapper jsonstream(jsonfilestream);
            mJson.ParseStream(jsonstream);
            for (const std::string& bindingName : mTypes) {
                const char* type = bindingName.c_str();
                if (mJson.HasMember(type)) {
                    const std::string& typePath = mJson[type].GetString();
                    textures.push_back(Texture<GL_TEXTURE_2D>(bindingName, typePath));
                }
            }
        }

        Material() = default;
        virtual ~Material() = default;

        void upload(Shader& shader, GLuint unitOffset = 0) {
            for (auto& texture : textures) {
                texture.upload(shader, unitOffset++);
            }
        }

    private:
        // JSON file path relative to the root project directory
        std::string mPath;

        // JSON container
        rapidjson::Document mJson;

        // Acceptable texture types
        static const size_t N_TEXTURE_TYPES = 6;
        const std::string mTypes[N_TEXTURE_TYPES] = {
            "diffuse",
            "specular",
            "normal",
            "ambientOcclusion",
            "emissive",
            "height"
        };

        // Capture textureIds for rendering
        std::vector< Texture<GL_TEXTURE_2D> > textures;
    };

    class Cubemap {
    public:
        Cubemap(const std::string& filename);

    };
}

#endif // MATERIALS_HPP
