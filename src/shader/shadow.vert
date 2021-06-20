#version 330 core
layout (location=0) in vec3 aPosition;
layout (location=1) in vec2 aTexcoord;
layout (location=2) in vec3 aNormal;

out VS_OUT{
    vec2 aTexcoord;
    vec4 worldPosition;
    vec3 worldNormal;
    vec4 projectPosition;
}vs_out;

uniform mat4 model, view, projection;

void main()
{
    vs_out.aTexcoord = aTexcoord;
    vs_out.worldNormal = vec3(transpose(inverse(model)) * vec4(aNormal, 1.f));
    vs_out.worldPosition = model * vec4(aPosition, 1.f);
    vs_out.projectPosition  = projection * view * vs_out.worldPosition;
    gl_Position = vs_out.projectPosition;
}