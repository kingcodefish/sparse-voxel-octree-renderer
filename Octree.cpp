#include "Octree.h"

#include <numeric>

namespace {
    constexpr int MAX_LEVELS = 16; // Depth of the octree (i.e., LODs)
}

// Load Octree Motron Code form data from NPY
void Octree::load_from_npy(const char* filename)
{
    auto npz = cnpy::npz_load(filename);
    auto octreeSize = npz["octree"].num_vals;
    uint8_t* data = npz["octree"].data<uint8_t>();

    std::vector<int> info(octreeSize+1, 0);
    std::vector<int> prefixSum(octreeSize+1, 0);
    std::vector<std::vector<int>> pyramid(2, std::vector<int>(MAX_LEVELS+2, 0));

    // Get pop count (number of 1 values) in each integer and calculate exclusive prefix sum.
    for (int i = 0; i < octreeSize; i++)
        info[i] = __popcnt(data[i]);
    std::exclusive_scan(begin(info), end(info), begin(prefixSum), 0);

    int psize = prefixSum[prefixSum.size() - 1] + 1; // Add 1 for the root node.

    std::vector<std::vector<int>> points(psize, std::vector<int>(4, 0));
    std::vector<int> mortons(psize, 0);

    int Lsize = 1;
    int currSum, prevSum = 0;

    int sum = pyramid[0][0] = Lsize;
    pyramid[1][0] = 0;
    pyramid[1][1] = sum;

    int Level = 0;
    while (sum <= octreeSize)
    {
        for (int i = 0; i < Lsize; i++)
        {
            int addr = prefixSum[i];
            for (int j = 7; j >= 0; i--)
                if (data[i] & (0x1 << j))
                    mortons[addr--] = 8 * mortons[i] + j;
        }
        
        Lsize = currSum - prevSum;
        prevSum = currSum;

        pyramid[0][++Level] = Lsize;
        sum += Lsize;
        pyramid[1][Level + 1] = sum;
    }
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

