#ifndef MATERIALS_HPP
#define MATERIALS_HPP

#include <string>

namespace XK {
    /**
     * Material should be able to handle the metallic/roughness workflow necessary for PBR
     * - ALBEDO
     * - NORMAL MAP
     * -
     */
    class Material {
    private:

    public:
        Material();
        virtual ~Material() = default;



    private:

    };

    class Cubemap {
    public:
        Cubemap(const std::string& filename);


    };
}

#endif // MATERIALS_HPP
