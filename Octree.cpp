#include "Octree.h"

namespace {
    constexpr int MAX_LEVELS = 16; // Depth of the octree (i.e., LODs)
}

// Load Octree Motron Code form data from NPY
void Octree::load_from_npy(const char* filename)
{
    data = cnpy::npy_load(filename);
    //uint8_t* loaded_data = arr.data<uint8_t>();

    //return convert_to_buffer(loaded_data);
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

