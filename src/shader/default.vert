#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTexcoord;
layout (location=2) in vec3 aNormal;
layout (location=3) in vec3 aTangent;
out vec4 VertexPos;
out vec2 texcoord;
out vec3 normal;
out vec3 tangent;
uniform mat4 model, view, project;

void main()
{
    texcoord = aTexcoord;
    normal = aNormal;
    tangent = aTangent;
    VertexPos = project * view * model * vec4(aPos.xyz, 1.f);
    gl_Position = VertexPos;
}