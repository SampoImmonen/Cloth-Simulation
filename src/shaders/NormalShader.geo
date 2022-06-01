#version 430

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float normal_lenght = 0.5f;

uniform mat4 projection;

void GenerateLine(int index){
    gl_Position = projection*gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection*(gl_in[index].gl_Position+vec4(gs_in[index].normal, 0.0)*normal_lenght);
    EmitVertex();
    EndPrimitive();
}

void main(){
    GenerateLine(0);
    GenerateLine(1);
    GenerateLine(2);
}