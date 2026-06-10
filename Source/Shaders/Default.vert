#version 450

layout(set = 1, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 1, binding = 1) uniform ModelToWorldBufforObject
{
    mat4 modelToWorld;
} modelUBO;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outFragPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outFragTexCoord;

void main() 
{
	gl_Position = ubo.proj * ubo.view * modelUBO.modelToWorld * vec4(inPosition, 1.0);
    
    outNormal = mat3(transpose(inverse(modelUBO.modelToWorld))) * inNormal;
    outFragPos = vec3(modelUBO.modelToWorld * vec4(inPosition, 1.0));
    
    outFragTexCoord = inTexCoord;
}