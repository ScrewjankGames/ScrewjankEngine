#version 450

layout(set = 1, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 fragTexCoord;

void main() 
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    outNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    fragTexCoord = inTexCoord;
}