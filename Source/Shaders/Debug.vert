#version 450

layout(set = 1, binding = 0) uniform GlobalUniformBufferObject {
    mat4 view;
    mat4 proj;
} gUbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;

void main() 
{
	gl_Position = gUbo.proj * gUbo.view * vec4(inPosition, 1.0);
    outColor = inColor;
}