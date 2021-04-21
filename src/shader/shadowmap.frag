#version 330 core
in VS_OUT{
    vec4 fragPos;// view pos
}fs_in;

uniform struct Light
{
    vec4 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
    float cutoff, outerCutoff;
}light;

void main()
{
    
}