#version 330 core
layout (location=0) in vec3 aPosition;
layout (location=1) in vec2 aTexcoord;
layout (location=2) in vec3 aNormal;
layout (location=3) in vec3 aTangent;

uniform mat4 model, view, projection;

out VS_OUT{
    vec3 aPosition;
    vec2 aTexcoord;
    vec3 aNormal;
    vec3 aTangent;
    vec4 worldPosition; // world pos
    mat3 worldTBN;
} vs_out;

mat3 CalcWorldTBN(mat4 model, vec3 tangent, vec3 normal)
{
    mat3 normal_model = transpose(inverse(mat3(model)));
    vec3 T = normalize(normal_model*tangent);
    vec3 N = normalize(normal_model*normal);
    vec3 B = cross(T,N);
    return mat3(T,B,N);
}

void main()
{
    vs_out.aPosition = aPosition;
    vs_out.aTexcoord = aTexcoord; 
    vs_out.aNormal = aNormal; 
    vs_out.aTangent = aTangent;

    // calculate worldTBN
    vs_out.worldTBN = CalcWorldTBN(model, aTangent, aNormal);
    
    // calculate vertex position
    vs_out.worldPosition =  model * vec4(aPosition.xyz, 1.f);
    gl_Position = projection * view * vs_out.worldPosition;
}