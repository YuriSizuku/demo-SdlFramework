#version 330 core
in VS_OUT{
    vec2 aTexcoord;
    vec4 worldPosition;
    vec3 worldNormal;
    vec2 ndcCoord;
}fs_in;

uniform sampler2D rednerTexture;
uniform sampler2D shadowMap2D; 
uniform samplerCube shadowMapCube;
uniform mat4 lightMatrix;
uniform struct Light {
    vec4 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
    float cutoff, outerCutoff;
}light;
uniform float biasMin, biasMax;

out vec4 FragColor;

void main()
{
    float depth = 0.f;
    float shadow = 0.f;
    float bias = 0.f;
    vec3 shadowMapCoord = vec3(0.f);
    if(abs(light.position.w) <= 0.001f) // direction light
    {
        vec4 tcoord = lightMatrix * fs_in.worldPosition;
        shadowMapCoord = vec3((tcoord.xyz/tcoord.w + 1.f)/2.f);
        depth = texture(shadowMap2D, shadowMapCoord.xy).r; // because of depth map, use only r
        bias = max(biasMax*(1.f - dot(light.direction, fs_in.worldNormal)), biasMin);
        shadow = shadowMapCoord.z  - bias < depth ? 0.f : 1.f;
    }
    else if(light.cutoff>0.001f)  // spot light
    {
        vec4 tcoord = lightMatrix * fs_in.worldPosition;
        shadowMapCoord = vec3((tcoord.xyz/tcoord.w + 1.f)/2.f);
        depth = texture(shadowMap2D, shadowMapCoord.xy).r;
        bias = max(biasMax*(1.f - dot(light.direction, fs_in.worldNormal)), biasMin);
        shadow = shadowMapCoord.z  - bias < depth ? 0.f : 1.f;
    }
    else // point light
    {
        shadowMapCoord = vec3(fs_in.worldPosition - light.position);
        texture(shadowMapCube, shadowMapCoord);
    }
    FragColor = vec4(vec3(1-shadow), 1.f) * texture(rednerTexture, fs_in.ndcCoord.xy*0.5f + 0.5f);
}