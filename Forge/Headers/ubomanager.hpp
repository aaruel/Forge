#ifndef UBOMANAGER_HPP
#define UBOMANAGER_HPP

#include <math.h>
#include <string>
#include <tuple>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include "shadermanager.hpp"

// This defines types that comply with std140
namespace UBO {
    using Float = glm::f32;
    using Vector = glm::vec4;
    using Matrix = glm::mat4;
};

namespace XK {
    template<typename ...T>
    class UBOManager : public ShaderListener {
    private:
        constexpr size_t typeSum() {
            return (sizeof(T) + ...);
        }

        template<size_t ...I>
        constexpr void setOffsets_impl(std::index_sequence<I...>) {
            size_t offset = 0;
            ((mOffsets[I] = offset, offset += sizeof(T)), ...);
        }

        template< typename I = std::make_index_sequence<sizeof...(T)> >
        constexpr void setOffsets() {
            setOffsets_impl(I{});
        }

    public:
        UBOManager<T...>(const std::string name) :
            mName(name),
            mSize(typeSum()),
            mNumParams(sizeof...(T)),
            mBindingOffset(gBindingOffset++)
        {
            // Setup the array of offsets to control where data is loaded into the UBO
            setOffsets();

            // Allocate space for the ubo using the type parameter pack
            glGenBuffers(1, &mUBO);
            glBindBuffer(GL_UNIFORM_BUFFER, mUBO);
            glBufferData(GL_UNIFORM_BUFFER, mSize, nullptr, GL_STATIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        virtual void event(unsigned id) override {
            mShaderIds.push_back(id);
            unsigned bufferIndex = glGetUniformBlockIndex(id, mName.c_str());
            glUniformBlockBinding(id, bufferIndex, mBindingOffset);
        }

        template<size_t N, typename IT>
        void set(IT& input) {
            // Ensure the input is matching up with the correct slot
            static_assert(
                std::is_same<decltype(std::get<N>(mTypeBuf)), IT>::value,
                "The input does not match the declared type at index N"
            );

            const size_t offset_begin = mOffsets[N];
            const size_t offset_end = sizeof(IT);
            glBindBuffer(GL_UNIFORM_BUFFER, mUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, offset_begin, offset_end, glm::value_ptr(input));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            glBindBufferBase(GL_UNIFORM_BUFFER, mBindingOffset, mUBO);
        }

    private:
        // Buffer specific variables
        const std::string mName;
        const size_t mSize;
        const size_t mNumParams;
        GLuint mUBO;
        GLuint mOffsets[sizeof...(T)] = {0};
        std::tuple<T...> mTypeBuf;
        std::vector<unsigned> mShaderIds;

        // Global UBO variables
        GLuint mBindingOffset;
        static GLuint gBindingOffset;
    };

    // OOL Initializers
    template<typename ...T>
    inline GLuint UBOManager<T...>::gBindingOffset = 0;
}

#endif // UBOMANAGER_HPP
