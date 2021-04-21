#version 330 core
in VS_OUT{
    vec4 fragPos;// view pos
}fs_in;
out vec4 FragColor;

uniform sampler2D rednerTexture;
uniform sampler2D shadowMap; 

void main()
{
    float depth = texture(rednerTexture, fs_in.fragPos.xy).r;
    FragColor = vec4(vec3(depth), 1.f);
    FragColor =  texture(shadowMap, fs_in.fragPos.xy);
}