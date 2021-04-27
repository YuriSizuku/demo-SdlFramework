#version 330 core
in VS_OUT{
    vec2 aTexcoord;
}fs_out;

uniform sampler2D defaultTexture;

out vec4 FragColor;

void main()
{
    FragColor = texture(defaultTexture, fs_out.aTexcoord);
}