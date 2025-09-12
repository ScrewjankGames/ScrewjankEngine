#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 vertexColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    //outColor = vec4(fragColor, 1.0f);
    outColor = texture(texSampler, vec2(fragTexCoord));
}