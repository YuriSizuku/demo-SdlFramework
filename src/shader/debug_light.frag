#version 300 es
precision mediump float;
uniform vec4 color;
out vec4 FragColor;

void main()
{
    FragColor = color;
}