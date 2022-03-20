//glsl version 4.5
#version 450

//shader input
layout (location = 0) in vec3 inColor;

//output write
layout (location = 0) out vec4 outFragColor;

layout(std140, set = 1, binding = 0) readonly buffer PointsBuffer{
	uint points[];
} pointsBuffer;

layout(std140, set = 1, binding = 1) readonly buffer PyramidBuffer{
	uint pyramid[];
} pyramidBuffer;

bool octreeHit(vec3 pos)
{
	return false;
}

int level = 5;

vec4 raytrace(vec3 pos, vec3 dir)
{
	uint osize = pyramidBuffer.pyramid[3];
	if (osize > 0)
		return vec4(0.0, 1.0, 0.0, 1.0);

	for (int i = 0; i < 50; i++)
	{
		if (octreeHit(pos))
			return vec4(1.0, 0.0, 0.0, 1.0);

		pos += dir * 1.0;
	}

	return vec4(0.0, 0.0, 0.0, 1.0);
}

void main()
{
	outFragColor = raytrace(vec3(0.5, 0.5, 0.0), vec3(0.0, 0.0, 1.0));
}
