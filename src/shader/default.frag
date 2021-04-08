#version 330 core
in vec4 VertexPos;
in vec2 texcoord;
in vec3 normal;
in vec3 tangent;
out vec4 FragColor;
void main()
{
    FragColor = vec4(VertexPos.xy, tangent.x, 1.f);
}