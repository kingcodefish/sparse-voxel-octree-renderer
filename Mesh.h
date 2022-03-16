#pragma once

#include "Types.h"
#include <vector>

#include <glm/vec3.hpp>

// Need to define this for the hash functions of GLM to be accessible.
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct VertexInputDescription {
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};


struct Vertex {

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;

	static VertexInputDescription get_vertex_description();

	bool operator==(const Vertex& other) const
	{
		return position == other.position && normal == other.normal && color == other.color;
	}
};

namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^
					(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
					(hash<glm::vec3>()(vertex.color) << 1);
		}
	};
}

struct Mesh {
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;

	AllocatedBuffer _vertexBuffer;
	AllocatedBuffer _indexBuffer;

	bool load_from_obj(const char* filename);
};
