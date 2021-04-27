#version 330 core
layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexcoord;
out VS_OUT{
    vec2 aTexcoord;
}vs_out;

uniform mat4 model, view, projection;

void main()
{
    vs_out.aTexcoord = aTexcoord;
    gl_Position = projection *  view * model * vec4(aPosition, 1.f);
}