//
//  renderable.hpp
//  Forge
//
//  Created by Aaron Ruel on 5/18/19.
//

#ifndef renderable_h
#define renderable_h

#include "shader.hpp"
#include "camera.hpp"
#include "utils.hpp"
#include <vector>
#include <tuple>
#include <kangaru/kangaru.hpp>

namespace XK {
    // Tuple vertex layout
    enum {
        position = 0,
        normal,
        uv,
        tangent,
        bitangent
    } TVertexEnum;
    
    using BasicVertex = std::tuple<
        glm::vec3, /* position  */
        glm::vec3  /* normal    */
    >;
    
    // Expansion if textures are present
    using TexVertexEx = std::tuple<
        glm::vec2  /* uv        */
    >;
    
    // Expansion if normals have tangent and bitangent vectors
    using NormalVertexEx = std::tuple<
        glm::vec3, /* tangent   */
        glm::vec3  /* bitangent */
    >;
    
    using TVertex = combined_tuple<BasicVertex, TexVertexEx, NormalVertexEx>;
    
    // Feed into enable_if_t to make sure the template parameter is a tuple
    template <typename T>
    constexpr bool IsTuple = false;
    template<typename ...types>
    constexpr bool IsTuple<std::tuple<types...>> = true;

    // Data for GPU uploading
    template<
        typename T,
        std::enable_if_t<IsTuple<T>>* = nullptr
    >
    struct RenderParams {
        std::vector<T> vertices;
        std::vector<GLuint> indices;
        GLuint vao;
        GLuint vbuffer;
        GLuint ibuffer;
    };

    class Renderable {
    private:
        template<size_t Idx, class T>
        constexpr size_t tuple_element_offset() {
            // Mimics the offsetof macro using a dummy tuple and some nasty casting
            T dummy;
            return static_cast<size_t>(
                reinterpret_cast<char*>(&std::get<Idx>(dummy))
                - reinterpret_cast<char*>(&std::get<0>(dummy))
            );
        }
    
        template<typename ...Ts, size_t ...I>
        void setAttribPointers_impl(
            std::index_sequence<I...>
        ) {
            
            // Set the pointer size based on the variadic template arguments of the tuple
            (glVertexAttribPointer(
                I,
                sizeof(Ts) / sizeof(GL_FLOAT),
                GL_FLOAT, GL_FALSE,
                sizeof(std::tuple<Ts...>),
                reinterpret_cast<GLvoid*>(tuple_element_offset<I, std::tuple<Ts...>>())
            ), ... );
            // Enable each vertex attrib array
            (glEnableVertexAttribArray(I), ... );
        }
        
        template<typename ...Ts>
        void setAttribPointers(const std::vector<std::tuple<Ts...>> &) {
            setAttribPointers_impl<Ts...>(std::index_sequence_for<Ts...> {});
        }
    
    public:
        // Render Execution Functions

        virtual void render() = 0;
        
        static void renderAll(std::vector<Renderable*> * v) {
            for (Renderable * r : *v) r->render();
        }
        
        // Getters / Setters

        void setShader(Shader * shader) { mShader = shader; }
        Shader * getShader() { return mShader; }
        
        void setCamera(Camera * camera) { mCamera = camera; }
        Camera * getCamera() { return mCamera; }
        
        // OpenGL Interface Functions

        template<typename T>
        void uploadToGPU(RenderParams<T> * rp) {
            // Bind a Vertex Array Object
            glGenVertexArrays(1, &rp->vao);
            glBindVertexArray(rp->vao);

            // Copy Vertex Buffer Data
            glGenBuffers(1, &rp->vbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, rp->vbuffer);
            glBufferData(
                GL_ARRAY_BUFFER,
                rp->vertices.size() * sizeof(typename decltype(rp->vertices)::value_type),
                &rp->vertices.front(), GL_STATIC_DRAW
            );

            // Copy Index Buffer Data
            glGenBuffers(1, &rp->ibuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rp->ibuffer);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                rp->indices.size() * sizeof(typename decltype(rp->indices)::value_type),
                &rp->indices.front(), GL_STATIC_DRAW
            );

            // Set Shader Attributes
            setAttribPointers(rp->vertices);

            // Cleanup Buffers
            glBindVertexArray(0);
            glDeleteBuffers(1, &rp->vbuffer);
            glDeleteBuffers(1, &rp->ibuffer);
        }

        // Model Transformation Functions

        virtual void translate(glm::vec3 coords) {
            mModel = glm::translate(mModel, coords);
        }

        virtual void rotate(glm::quat rot) {
            mRotation = glm::mat3_cast(rot);
            mModel = mRotation * mModel;
        }
        
    protected:
        // Rendering pointers for management
        Shader * mShader;
        Camera * mCamera;
        // Model matrix
        // Rotation * Transformation * Model
        glm::mat4 mModel = glm::mat4(1.f);
        glm::mat4 mRotation = glm::mat4(1.f);
        // Container for dependency injection
        kgr::container * mContainer;
        
    };
}

using Pipeline = std::vector<XK::Renderable*>;

#endif /* renderable_h */
