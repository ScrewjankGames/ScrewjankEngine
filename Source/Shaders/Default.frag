#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 error;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(fragColor, 1.0);
    if( error.x == 1.0f)
    {
        outColor = vec4(1,0,0,1);
    }
}