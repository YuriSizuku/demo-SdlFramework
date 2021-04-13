#version 330 core
#extension GL_NV_fragdepth : enable
in VS_OUT{
    vec3 aPos;
    vec4 aColor;
} fs_in;
out vec4 FragColor;

void main()
{
    gl_FragDepth = 0.f;
    FragColor = fs_in.aColor;
}