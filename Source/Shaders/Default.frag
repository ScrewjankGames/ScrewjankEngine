#version 450

layout(set = 2, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 inFragPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(texSampler, inFragTexCoord);
}