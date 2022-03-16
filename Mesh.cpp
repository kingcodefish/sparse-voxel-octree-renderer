#include "Mesh.h"
#include <tiny_obj_loader.h>
#include <iostream>
#include <unordered_map>

VertexInputDescription Vertex::get_vertex_description()
{
	VertexInputDescription description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(Vertex);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

	//Position will be stored at Location 0
	VkVertexInputAttributeDescription positionAttribute = {};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttribute.offset = offsetof(Vertex, position);

	//Normal will be stored at Location 1
	VkVertexInputAttributeDescription normalAttribute = {};
	normalAttribute.binding = 0;
	normalAttribute.location = 1;
	normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttribute.offset = offsetof(Vertex, normal);

	//Position will be stored at Location 2
	VkVertexInputAttributeDescription colorAttribute = {};
	colorAttribute.binding = 0;
	colorAttribute.location = 2;
	colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	colorAttribute.offset = offsetof(Vertex, color);

	description.attributes.push_back(positionAttribute);
	description.attributes.push_back(normalAttribute);
	description.attributes.push_back(colorAttribute);
	return description;
}

bool Mesh::load_from_obj(const char* filename)
{
	//attrib will contain the vertex arrays of the file
	tinyobj::attrib_t attrib;
	//shapes contains the info for each separate object in the file
	std::vector<tinyobj::shape_t> shapes;
	//materials contains the information about the material of each shape, but we wont use it.
	std::vector<tinyobj::material_t> materials;

	//error and warning output from the load function
	std::string err;

	//load the OBJ file
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename);
	//if we have any error, print it to the console, and break the mesh loading. 
	//This happens if the file cant be found or is malformed
	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!ret) {
		return false;
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	// Loop over shapes
	for (const auto& shape : shapes) {
		for (const auto& idx : shape.mesh.indices) {
			Vertex vertex{};

			//vertex position
			tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
			tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
			tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
			//vertex normal
			tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
			tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
			tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

			//copy it into our vertex
			Vertex new_vert;
			new_vert.position.x = vx;
			new_vert.position.y = vy;
			new_vert.position.z = vz;

			new_vert.normal.x = nx;
			new_vert.normal.y = ny;
			new_vert.normal.z = nz;

			//we are setting the vertex color as the vertex normal. This is just for display purposes
			new_vert.color = new_vert.normal;

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
				_vertices.push_back(vertex);
			}

			_indices.push_back(uniqueVertices[vertex]);
		}
	}

	return true;
}
