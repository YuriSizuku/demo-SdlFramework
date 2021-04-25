#version 330 core
#extension GL_NV_fragdepth : enable
in VS_OUT{
    vec4 viewPosition;
}fs_in;

uniform struct Light {
    vec4 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
    float cutoff, outerCutoff;
}light;

void main()
{
    if(abs(light.position.w) <= 0.001f) // direction light
    {
        gl_FragDepth = gl_FragCoord.z;
    }
    else if(light.cutoff>0.001f)  // spot light
    {
        gl_FragDepth = gl_FragCoord.z;
    }
    else // point light
    {

    }
}