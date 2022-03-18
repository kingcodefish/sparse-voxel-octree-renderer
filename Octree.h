#pragma once

#include <cnpy.h>

struct Octree {
    void load_from_npy(const char* filename);
    //bool convert_to_buffer(uint8_t* arr);

    std::vector<unsigned int> points;
    std::vector<unsigned int> pyramid;
};