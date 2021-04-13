#version 330 core
in VS_OUT
{
    vec3 aPos;
    vec2 aTexcoord;
    vec3 aNormal;
    vec3 aTangent;
    vec4 FragPos;
    mat3 worldTBN;
} fs_in;
out vec4 FragColor;

void main()
{
    vec3 N = fs_in.aNormal;
    FragColor = vec4((N+1.f)/2.f, 1.f);
}