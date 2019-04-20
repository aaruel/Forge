//
//  voxelfunctors.hpp
//  Forge
//
//  Created by Aaron Ruel on 4/6/19.
//

#ifndef voxelfunctors_h
#define voxelfunctors_h

#include <PolyVox/RawVolume.h>
#include <PolyVox/MaterialDensityPair.h>
#include <PolyVox/Raycast.h>
#include <PolyVox/PagedVolume.h>
#include <noise/noise.h>

using namespace PolyVox;

// A callback functor for raycasting
// Stores the result of a hit voxel and its adjacent voxel
class RaycastTest {
public:
    RaycastTest()
        : mValid(false)
    {}

    bool operator()(const PagedVolume<MaterialDensityPair88>::Sampler& sampler) {
        // Run until the ray hits a voxel with volume
        MaterialDensityPair88 current = sampler.getVoxel();
        
        if (current.getDensity() != MaterialDensityPair88::getMinDensity()) {
            mValid = true;
            // Stop iterating after finding valid voxel
            location = sampler.getPosition();
            return false;
        }
        
        // Save the previous block! Nice trick to detect an adjacent voxel
        prevLocation = sampler.getPosition();
        
        // We are in the volume, so decide whether to continue based on the voxel value.
        return true;
    }

    MaterialDensityPair88 mInvalidVoxel = MaterialDensityPair88(0, 0);
    bool mValid;
    Vector3DInt32 prevLocation;
    Vector3DInt32 location;
};

template<typename MDP>
class FastNoisePager : public PagedVolume<MDP>::Pager {
public:
    FastNoisePager() : PagedVolume<MDP>::Pager() {}
    virtual ~FastNoisePager() {};

    virtual void pageIn(
        const PolyVox::Region& region,
        typename PagedVolume<MDP>::Chunk* pChunk
    ) {
        using namespace noise;
        
        /// Improvement: Generate a height map and pass into constructor
        /// or an external generator
        
        // Mountain Range Area
        module::RidgedMulti mountain;
        mountain.SetOctaveCount(4);
        mountain.SetLacunarity(2.f);
        
        // Flat Area
        module::Billow baseFlat;
        module::ScaleBias flat;
        flat.SetSourceModule(0, baseFlat);
        flat.SetScale(0.125);
        flat.SetBias(-0.75);
        
        // Terrain Type Randomizer
        module::Perlin biome;
        biome.SetFrequency(0.5);
        biome.SetPersistence(0.25);
        
        // Final Stage
        module::Select mynoise;
        mynoise.SetSourceModule(0, flat);
        mynoise.SetSourceModule(1, mountain);
        mynoise.SetControlModule(biome);
        mynoise.SetEdgeFalloff(0.125);
        
        for (int x = region.getLowerX(); x <= region.getUpperX(); x++)
        for (int z = region.getLowerZ(); z <= region.getUpperZ(); z++) {
            float noiseVal = mynoise.GetValue(
                x / static_cast<float>(250),
                z / static_cast<float>(250),
                1.0
            );
            noiseVal *= 64;
            noiseVal += 100;
            for (int y = region.getLowerY(); y <= region.getUpperY(); y++) {
                float val = noiseVal - static_cast<float>(y); // val is positive when inside sphere
                val = clamp(val, -1.0f, 1.0f); // val is between -1.0 and 1.0
                val += 1.0f; // val is between 0.0 and 2.0
                val *= 127.5f; // val is between 0.0 and 255
            
                MaterialDensityPair88 voxel;
                voxel.setMaterial(1);
                voxel.setDensity(val);

                // Voxel position within a chunk always start from zero.
                // So if a chunk represents region (4, 8, 12) to (11, 19, 15)
                // then the valid chunk voxels are from (0, 0, 0) to (7, 11, 3).
                // Hence we subtract the lower corner position of the region from
                // the volume space position in order to get the chunk space position.
                pChunk->setVoxel(
                    x - region.getLowerX(),
                    y - region.getLowerY(),
                    z - region.getLowerZ(),
                    voxel
                );
            }
        }
    }

    virtual void pageOut(
        const PolyVox::Region& region,
        typename PagedVolume<MDP>::Chunk* /*pChunk*/
    ) {
        std::cout
            << "warning unloading region: "
            << region.getLowerCorner()
            << " -> "
            << region.getUpperCorner()
            << std::endl;
    }
};

#endif /* voxelfunctors_h */
