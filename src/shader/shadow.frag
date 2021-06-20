#version 330 core
in VS_OUT{
    vec2 aTexcoord;
    vec4 worldPosition;
    vec3 worldNormal;
    vec4 projectPosition;
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
uniform float shadowBrightness;

out vec4 FragColor;

void main()
{
    float depth = 0.f;
    float shadow = 0.f;
    float bias = max(biasMax*(1.f - dot(light.direction, fs_in.worldNormal)), biasMin);
    vec2 texelSize = 1.f / textureSize(rednerTexture, 0);
    vec3 shadowMapCoord = vec3(0.f);
 
    if(abs(light.position.w) <= 0.001f) // direction light
    {
        vec4 lightPosition = lightMatrix * fs_in.worldPosition;
        shadowMapCoord = vec3((lightPosition.xyz/lightPosition.w + 1.f)/2.f);
        for(int x=-1;x<=1;x++)   // PCF, percentage-closer filtering
        {
            for(int y=-1;y<=1;y++)
            {
                // use only r represent the depth in depth map
                depth = texture(shadowMap2D, shadowMapCoord.xy + texelSize*vec2(x,y)).r; 
                shadow += shadowMapCoord.z  - bias < depth ? 0.f : 1.f;
            }
        }
        shadow /= 9.f; 
    }
    else if(light.cutoff>0.001f)  // spot light
    {
        vec4 lightPosition = lightMatrix * fs_in.worldPosition;
        shadowMapCoord = vec3((lightPosition.xyz/lightPosition.w + 1.f)/2.f);
        depth = texture(shadowMap2D, shadowMapCoord.xy).r;
        shadow = shadowMapCoord.z  - bias < depth ? 0.f : 1.f;
    }
    else // point light
    {
        shadowMapCoord = vec3(fs_in.worldPosition - light.position);
        texture(shadowMapCube, shadowMapCoord);

        vec3 sampleOffsetDirections[20] = vec3[](
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1));
    }

    // can not divide in vertex, or will be cliped if out of range
    vec2 ndcCoord = fs_in.projectPosition.xy / fs_in.projectPosition.w; 
    FragColor = vec4(0.f);
    FragColor = vec4(vec3(max(1-shadow, shadowBrightness)), 1.f) * texture(rednerTexture,  ndcCoord.xy*0.5f + 0.5f);
}