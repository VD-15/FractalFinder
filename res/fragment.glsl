#version 430

layout(location = 0) in vec2 fragUV;
layout(location = 0) uniform sampler2D ourTexture;
layout(location = 2) uniform vec4 color;

out vec4 fragColor;

void main()
{
	fragColor = texture(ourTexture, fragUV) * color;
	//fragColor = vec4(fragUV.xy, 0.0, 1.0);
}
