#version 430

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 view;
uniform mat4 model;

out VS_OUT{
    vec3 normal;
} vs_out;

void main(){
    gl_Position = view*model*vec4(aPos, 1.0f);
    mat3 normalMatrix = mat3(transpose(inverse(view*model)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix*aNormal, 0.0f)));
}