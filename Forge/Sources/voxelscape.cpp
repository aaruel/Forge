//
//  voxelscape.cpp
//  Forge
//
//  Created by Aaron Ruel on 5/28/19.
//

#include "voxelscape.hpp"

#include <stddef.h>
#include <glm/gtc/type_ptr.hpp>

namespace XK {
    void VoxelPlane::GetSurface(
        float xStart, float xEnd, float xStep,
        float yStart, float yEnd, float yStep,
        float zStart, float zEnd, float zStep,
        float* output, unsigned char* materialid, unsigned char* blend
    ) {
        int id = 0;
        for (float z = zStart; z < zEnd; z += zStep)
        for (float y = yStart; y < yEnd; y += yStep)
        for (float x = xStart; x < xEnd; x += xStep) {
            float dist = glm::length( glm::vec3(x,y,z) );
            output[id] = 10.f-dist;
            if (materialid == nullptr) {
                materialid[id] = 0;
                blend[id] = 0;
            }
            id++;
        }
    }
}
