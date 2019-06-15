//
//  voxelscape.cpp
//  Forge
//
//  Created by Aaron Ruel on 5/28/19.
//

#include "voxelscape.hpp"

#include <stddef.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

namespace XK {
    void VoxelPlane::GetSurface(
        float xStart, float xEnd, float xStep,
        float yStart, float yEnd, float yStep,
        float zStart, float zEnd, float zStep,
        float* output, unsigned char* materialid, unsigned char* blend
    ) {
        glm::vec3 plane = glm::vec3(0.f, 0.f, 1.f);
        int id = 0;
        const int d = 2;
        for (float z = zStart/d; z < zEnd/d; z += zStep/d)
        for (float y = yStart/d; y < yEnd/d; y += yStep/d)
        for (float x = xStart/d; x < xEnd/d; x += xStep/d) {
            glm::vec3 point = glm::vec3(x,y,z);
            float density = glm::dot(plane, point);
            density += glm::perlin(point * 0.25f) * 7.5f * 4.f;
            output[id] = density;
            if (materialid == nullptr) {
                materialid[id] = 0;
                blend[id] = 0;
            }
            ++id;
        }
    }
}
