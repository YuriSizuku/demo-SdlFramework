#version 330 core
layout (location=0) in vec3 aPosition;
out VS_OUT{
    vec4 viewPosition;
}vs_out;

uniform mat4 model, view, projection;

void main()
{
    vs_out.viewPosition = view * model * vec4(aPosition, 1.f);
    gl_Position = projection * vs_out.viewPosition;
}