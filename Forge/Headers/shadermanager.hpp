#ifndef SHADERMANAGER_HPP
#define SHADERMANAGER_HPP

#include <vector>
#include "shader.hpp"

namespace XK {
    template<typename ...T>
    class Listener {
    public:
        virtual void event(T...) {}
        virtual ~Listener() = default;
    };

    using ShaderListener = Listener<unsigned>;

    class ShaderManager {
    private:
        void emit_newShader(unsigned id) {
            for (ShaderListener& listener : mListeners) {
                listener.event(id);
            }
        }

        // Push existing shaders to the listening class
        void provide(ShaderListener& listener) {
            for (auto& shader : mShaders) {
                listener.event(shader.get());
            }

        }

    public:
        template<typename T>
        void attach(T&& shader) {
            mShaders.push_back(std::forward(shader));
            emit_newShader(shader.get());
        }

        void listen(ShaderListener& listener) {
            mListeners.push_back(listener);
            provide(listener);
        }

    private:
        std::vector<Shader> mShaders;
        // Uses the unsigned type parameter as it only returns the shader id
        std::vector< std::reference_wrapper<ShaderListener> > mListeners;

    };
}

#endif // SHADERMANAGER_HPP
