#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTexcoord;
layout (location=2) in vec3 aNormal;
layout (location=3) in vec3 aTangent;

uniform mat4 model, view, project;

out VS_OUT
{
    vec2 model_texcoord;
    vec3 model_normal;
    vec3 model_tangent;
    vec4 FragPos;
    mat3 worldTBN;
} vs_out;

mat3 calculate_worldTBN(mat4 model, vec3 tangent, vec3 normal)
{
    mat3 normal_model = transpose(inverse(mat3(model)));
    vec3 T = normalize(normal_model*tangent);
    vec3 N = normalize(normal_model*normal);
    vec3 B = cross(T,N);
    return mat3(T,B,N);
}

void main()
{
    vs_out.model_texcoord = aTexcoord; 
    vs_out.model_normal = aNormal; 
    vs_out.model_tangent = aTangent;

    // calculate worldTBN
    vs_out.worldTBN = calculate_worldTBN(model, aTangent, aNormal);
    
    // calculate vertex position
    vs_out.FragPos = project * view * model * vec4(aPos.xyz, 1.f);
    gl_Position = vs_out.FragPos;
}