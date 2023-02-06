#version 300 es
layout (location=0) in vec3 aPos;

uniform mat4 model, view, projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.f);
}