#version 330 core
in vec4 VertexPos;
in vec2 texcoord;
in vec3 normal;
in vec3 tangent;
out vec4 FragColor;
void main()
{
    FragColor = vec4((tangent+1.f)/2.f, 1.f);
    FragColor = vec4((normal+1.f)/2.f, 1.f);
    //FragColor = vec4(1.f, 0.f, 1.f, 1.f);
}