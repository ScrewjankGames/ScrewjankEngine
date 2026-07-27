#version 450

layout(set = 2, binding = 0) uniform sampler2D albedoTexture;

layout(set = 3, binding = 0) uniform DefaultMaterialSettings
{
    vec4 baseAlbedoColor;
    uint useTexSampler;
}
material;

layout(location = 0) in vec3 inFragPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vec3 lightPos = vec3(0,0,0);
    vec3 lightDir = normalize(lightPos - inFragPos);

    float ambientStrength = 0.1;
    vec4 ambientColor = ambientStrength * lightColor;

    vec4 albedoColor = material.baseAlbedoColor;

    if(material.useTexSampler == 1)
        albedoColor = albedoColor * texture(albedoTexture, inFragTexCoord);

    float diffuseAmt = max(dot(inNormal, lightDir), 0.0);
    vec4 diffuseColor = diffuseAmt * lightColor;

    outColor = (ambientColor + diffuseColor) * albedoColor;
    //outColor = albedoColor;
}