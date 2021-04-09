#version 330 core
in VS_OUT
{
    vec2 model_texcoord;
    vec3 model_normal;
    vec3 model_tangent;
    vec4 FragPos;
    mat3 worldTBN;
} fs_in;
out vec4 FragColor;

void main()
{
    vec3 N = fs_in.model_normal;
    FragColor = vec4((N+1.f)/2.f, 1.f);
}