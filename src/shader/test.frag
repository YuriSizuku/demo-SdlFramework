#version 330 core
in VS_OUT{
    vec3 aPos;
    vec2 aTexcoord;
    vec3 aNormal;
    vec3 aTangent;
    vec4 FragPos;
    mat3 worldTBN;
} fs_in;

#define MAX_LIGHTS 20

layout (std140) uniform BLOCK1
{
    float alpha;
};
uniform sampler2D defaultTexture;
uniform vec3 viewPos;
uniform struct Light
{
    vec4 position;
    vec3 color;
    vec3 attenuation;
    vec3 spotDirection;
    float cutoff, outerCutoff;
}lights[MAX_LIGHTS];
uniform struct MaterialPhong 
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}  material;

out vec4 FragColor;

void main()
{
    vec3 N = fs_in.aNormal;
    FragColor = texture(defaultTexture, fs_in.aTexcoord);
    //FragColor = vec4((N+1.f)/2.f, 1.f);
    //FragColor = vec4(1.f, 1.f, 1.f, alpha);
}