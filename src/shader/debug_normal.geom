#version 330 core
#extension GL_EXT_geometry_shader : enable
#extension GL_OES_geometry_shader : enable
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in VS_OUT
{
    vec3 viewNormal;
} gs_in[];
uniform mat4 projection;

void emit_line(int index, vec3 direction)
{
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * (gl_in[index].gl_Position + vec4(direction, 0.f));
    EmitVertex();
    EndPrimitive();
}

const float MAGNITUDE = 0.1;

void main()
{
    for(int i=0;i<3;i++)
    {
        emit_line(i,MAGNITUDE*gs_in[i].viewNormal); // generate the normal line
    }
}