#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTexcoord;
layout (location=2) in vec3 aNormal;
layout (location=3) in vec3 aTangent;

uniform mat4 model, view;

out VS_OUT
{
    vec3 viewNormal;
} vs_out;

void main()
{
    mat3 normalMat = mat3(transpose(inverse(view*model)));
    vs_out.viewNormal = normalize(normalMat * aNormal);
    gl_Position = view * model * vec4(aPos.xyz, 1.f);
}