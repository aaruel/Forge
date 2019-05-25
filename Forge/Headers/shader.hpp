#ifndef shader_h
#define shader_h
#pragma once

// System Headers
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Standard Headers
#include <string>
#include <vector>

// Define Namespace
namespace XK
{
    class Shader
    {
        using Paths = std::vector<std::string>;
    public:
    
        static std::vector<Shader*> getAllShaders();
        static void addToAllShaders(Shader*);

        // Implement Custom Constructor and Destructor
         Shader() { mProgram = glCreateProgram(); addToAllShaders(this); }
        ~Shader() { glDeleteProgram(mProgram); }

        // Public Member Functions
        Shader & activate();
        Shader & attach(std::string const & filename);
        GLuint   create(std::string const & filename);
        GLuint   get() { return mProgram; }
        Shader & link();
        Shader & reload();
        Paths    getPaths() { return mPaths; }

        // Wrap Calls to glUniform
        void bind(unsigned int location, float value);
        void bind(unsigned int location, int value);
        void bind(unsigned int location, glm::mat4 const & matrix);
        void bind(unsigned int location, glm::vec3 const & vec);
        template<typename T> Shader & bind(std::string const & name, T&& value)
        {
            int location = glGetUniformLocation(mProgram, name.c_str());
            if (location == -1) ;// fprintf(stderr, "Missing Uniform: %s\n", name.c_str());
            else bind(location, std::forward<T>(value));
            return *this;
        }

    private:

        // Disable Copying and Assignment
        Shader(Shader const &) = delete;
        Shader & operator=(Shader const &) = delete;

        // Private Member Variables
        GLuint mProgram;
        GLint  mStatus;
        GLint  mLength;
        Paths  mPaths;

    };
};

#endif /* shader_h */
