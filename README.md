# NGLoD Sparse Voxel Octree Renderer

Eventually will be a Vulkan-based implementation Sparse Voxel Octree Renderer of the Neural Geometric Level of Detail octree representation.

To generate build files, create a build folder, cd into it, and run:
```
conan install --build missing ..
cmake ..
```

All SPIR-V files are built automatically as a post-build step in CMake.
