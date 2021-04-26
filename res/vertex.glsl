#version 430

layout(location = 1) uniform mat3 proj;

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

void main()
{
	vec3 t = vec3(inPos, 1.0) * proj;
	gl_Position = vec4(t, 1.0);
	fragUV = inUV;
}
