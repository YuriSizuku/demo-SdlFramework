#version 330 core
layout (location=0) in vec3 aPosition;
layout (location=1) in vec2 aTexcoord;
layout (location=2) in vec3 aNormal;

out VS_OUT{
    vec2 aTexcoord;
    vec4 worldPosition;
    vec3 worldNormal;
    vec2 ndcCoord;
}vs_out;

uniform mat4 model, view, projection;

void main()
{
    vs_out.aTexcoord = aTexcoord;
    vs_out.worldNormal = vec3(transpose(inverse(model)) * vec4(aNormal, 1.f));
    vs_out.worldPosition = model * vec4(aPosition, 1.f);
    gl_Position = projection * view * vs_out.worldPosition;
    vs_out.ndcCoord = gl_Position.xy / gl_Position.w; 
}