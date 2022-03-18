#include "Octree.h"

#include <glm/vec3.hpp>

#include <numeric>

#pragma optimize("", off)

namespace {
    constexpr int MAX_LEVELS = 16; // Depth of the octree (i.e., LODs)
}

// Load Octree Motron Code form data from NPY
void Octree::load_from_npy(const char* filename)
{
    auto npz = cnpy::npz_load(filename);
    auto octreeSize = npz["octree"].num_vals;
    uint8_t* data = npz["octree"].data<uint8_t>();

    std::vector<unsigned int> info(octreeSize+1, 0);
    std::vector<unsigned int> prefixSum(octreeSize+1, 0);
    std::vector<std::vector<unsigned int>> pyramidLevels(2, std::vector<unsigned int>(MAX_LEVELS+2, 0));

    // Get pop count (number of 1 values) in each integer and calculate exclusive prefix sum.
    for (int i = 0; i < octreeSize; i++)
        info[i] = __popcnt(data[i]);
    std::exclusive_scan(begin(info), end(info), begin(prefixSum), 0);

    int psize = prefixSum[prefixSum.size() - 1] + 1; // Add 1 for the root node.

    std::vector<unsigned int> mortons(psize, 0);

    int Lsize = 1;
    int currSum = 0, prevSum = 0;

    int sum = pyramidLevels[0][0] = Lsize;
    pyramidLevels[1][0] = 0;
    pyramidLevels[1][1] = sum;

    // Honestly, your guess is as good as mine...
    // Some of this seems suboptimal.... hmmm
    int Level = 0;
    while (sum <= octreeSize)
    {
        for (int i = 0; i < Lsize; i++)
        {
            int addr = prefixSum[i+1];
            int code = mortons[i];
            for (int j = 7; j >= 0; j--)
                if (data[i] & (0x1 << j))
                    mortons[addr--] = 8 * code + j;
        }

        currSum = prefixSum[prevSum + 1];
        Lsize = currSum - prevSum;
        prevSum = currSum;

        pyramidLevels[0][++Level] = Lsize;
        sum += Lsize;
        pyramidLevels[1][Level + 1] = sum;
    }

    int totalPoints = pyramidLevels[1][Level + 1];

    for (int i = 0; i < totalPoints; i++)
    {
        glm::ivec3 p{0, 0, 0};
        for (int j = 0; j < 16; j++)
        {
            p.x |= (mortons[i] & (0x1ull << (3 * i + 2))) >> (2 * i + 2);
            p.y |= (mortons[i] & (0x1ull << (3 * i + 1))) >> (2 * i + 1);
            p.z |= (mortons[i] & (0x1ull << (3 * i + 0))) >> (2 * i + 0);
        }
        points.push_back(p.x);
        points.push_back(p.y);
        points.push_back(p.z);
    }

    pyramid.reserve((Level + 2) * 2);
    pyramid.insert(pyramid.begin(), pyramidLevels[0].begin(), pyramidLevels[0].begin() + Level + 2);
    pyramid.insert(pyramid.begin(), pyramidLevels[1].begin(), pyramidLevels[1].begin() + Level + 2);
}

// Converts the NPY data buffer to its Kaolin octree structure representation.
// Reference: https://kaolin.readthedocs.io/en/v0.9.1/modules/kaolin.ops.spc.html
// Tutorial: https://github.com/NVIDIAGameWorks/kaolin/blob/master/examples/tutorial/understanding_spcs_tutorial.ipynb
/*bool Octree::convert_to_buffer(uint8_t* arr)
{
    // The input array contains an octree represented as a tensor of bytes.
    // Each bit in the byte array represents the binary occupancy of an
    // octree bit in Morton order. The bytes are in BFS order.


    return false;
}*/

