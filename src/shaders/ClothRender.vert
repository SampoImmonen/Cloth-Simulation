#version 430

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

out vec3 normal;
out vec2 texCoords;
out vec3 fragPos;
out vec4 fragPosLightSpace;
out mat3 TBN;

void main()
{
    normal = mat3(transpose(inverse(model)))*aNormal;
    texCoords = aTexCoords;
    fragPos = vec3(model*vec4(aPos, 1.0f));
    fragPosLightSpace = lightSpaceMatrix*vec4(fragPos, 1.0);
    gl_Position = projection*view*model*vec4(aPos,1.0);

    vec3 T = normalize(vec3(model*vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model*vec4(aNormal, 0.0)));
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
}