#version 430

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

out vec3 normal;
out vec2 texCoords;
out vec3 fragPos;
out vec4 fragPosLightSpace;

void main()
{
    normal = mat3(transpose(inverse(model)))*aNormal;
    texCoords = aTexCoords;
    fragPos = vec3(model*vec4(aPos, 1.0f));
    fragPosLightSpace = lightSpaceMatrix*vec4(fragPos, 1.0);
    gl_Position = projection*view*model*vec4(aPos,1.0);
}