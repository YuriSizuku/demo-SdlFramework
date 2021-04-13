#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aTexcoord;

uniform mat4 model, view, project;

out VS_OUT{
    vec3 aPos;
    vec4 aColor;
} vs_out;

void main()
{
    vs_out.aPos = aPos;
    float r = floor(aTexcoord.s/1000.f);
    float g = aTexcoord.s-1000.f*r;
    float b = floor(aTexcoord.t/1000.f);
    float a = aTexcoord.t-1000.f*b;
    vs_out.aColor = vec4(r,g,b,a)/999.f;
    gl_Position = project * view * model * vec4(vs_out.aPos, 1.f);
}