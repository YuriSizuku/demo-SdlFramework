#version 300 es
in vec3 aPosition;
in vec2 aTexcoord;
in vec3 aNormal;
in vec3 aTangent;

uniform mat4 model, view, projection;

// out VS_OUT{
out vec3 position;
out vec2 texcoord;
out vec3 normal;
out vec3 tangent;
out vec4 worldPosition;
out mat3 worldTBN;
// } vs_out;

mat3 CalcWorldTBN(mat4 model, vec3 tangent, vec3 normal)
{
    mat3 normal_model = transpose(inverse(mat3(model)));
    vec3 T = normalize(normal_model*tangent);
    vec3 N = normalize(normal_model*normal);
    vec3 B = cross(T,N);
    return mat3(T,B,N);
}

void main()
{
    position = aPosition;
    texcoord = aTexcoord; 
    normal = aNormal; 
    tangent = aTangent;

    // calculate worldTBN
    worldTBN = CalcWorldTBN(model, aTangent, aNormal);
    
    // calculate vertex position
    worldPosition =  model * vec4(aPosition.xyz, 1.f);
    gl_Position = projection * view * worldPosition;
}