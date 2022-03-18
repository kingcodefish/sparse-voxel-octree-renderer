//glsl version 4.5
#version 450

//shader input
layout (location = 0) in vec3 inColor;

//output write
layout (location = 0) out vec4 outFragColor;

layout(std140, set = 1, binding = 0) readonly buffer OctreeBuffer{
	uint octreeData[];
} octreeBuffer;

bool octreeHit(vec3 pos)
{
	return false;
}

vec4 raymarch(vec3 pos, vec3 dir)
{
	for (int i = 0; i < 10; i++)
	{
		if (octreeHit(pos))
			return vec4(1.0, 0.0, 0.0, 1.0);

		pos += dir * 10;
	}

	return vec4(0.0, 0.0, 0.0, 1.0);
}

void main()
{
	outFragColor = raymarch(vec3(0.5, 0.5, 0.0), vec3(0.0, 0.0, 0.0));
}
