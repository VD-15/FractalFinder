#version 430

#define NUM_ITERATIONS 300

layout(local_size_x = 1, local_size_y = 1) in;

layout(rgba32f, binding = 0) uniform image2D destTex;
layout(location = 1) uniform int numIterations;
layout(location = 2) uniform float size;
layout(location = 3) uniform vec2 offset;

vec2 ComplexSquare(vec2 c)
{
	return vec2(c.x * c.x - c.y * c.y, 2.0 * c.x * c.y);
}

vec2 ComplexAdd(vec2 r, vec2 l)
{
	return vec2(r.x + l.x, r.y + l.y);
}

int Iterate(vec2 c)
{
	vec2 z = vec2(0.0, 0.0);
	int escape = 0;

	for (int i = 0; i < numIterations; i++)
	{
		z = ComplexAdd(ComplexSquare(z), c);
		
		escape = max(int(length(z) > 2.0) * int(escape == 0) * i, escape);
		//escape = (length(z) > 2.0) ? escape : min(escape, 1);
	}

	return escape;
}

vec3 HueToRGB(float hue)
{
	vec3 c;
	c.x = abs(hue * 6 - 3) - 1;
	c.y = 2 - abs(hue * 6 - 2);
	c.z = 2 - abs(hue * 6 - 4);
	return c;
	return normalize(c);
}

void main()
{
	// Pixels we're writing to
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

	vec2 bounds = vec2(gl_NumWorkGroups.xy);
	vec2 imagePos = vec2(storePos);
	float aspect = bounds.x / bounds.y;

	vec2 worldPos = vec2(
		mix(offset.x - size, offset.x + size, imagePos.x / bounds.x),
		mix(offset.y - size, offset.y + size, imagePos.y / bounds.y)
	);

	vec2 complex = worldPos;

	int result = Iterate(complex);

	vec3 value = result > 0 ? HueToRGB(mod(float(result) / 50, 1.0)) : vec3(0.0, 0.0, 0.0);
	
	imageStore(destTex, storePos, vec4(value, 1.0));
}
