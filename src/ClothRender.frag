#version 430

const float PI = 3.14159265359;

out vec4 FragColor;

in vec3 normal;
in vec2 texCoords;
in vec3 fragPos;

uniform vec3 colordebug;
uniform float exposure = 1.0f;
uniform vec3 viewPos;

//material uniforms
uniform sampler2D Tex;

struct SurfaceProperties{
    vec3 albedo;
	float metallic;
	float roughness;
	float ao;
};

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;

    bool hasAlbedo;

    sampler2D albedoMap;
};

struct DirLight {
    vec3 direction;
    vec3 intensity;

    float size;
    bool shadows;
    sampler2D shadowMap;
};

uniform Material material;
uniform DirLight dirlight;

//functions used for cook-terrance BRDF
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 CalcDirectionalLight(DirLight light, vec3 N, vec3 V, vec3 F0, SurfaceProperties props) {
	vec3 L = normalize(light.direction);
	vec3 H = normalize(V + L);
	//float distance = length(light.position - fragPos);
	vec3 radiance = light.intensity;

	vec3 F = fresnelSchlick(max(dot(H, V), 0), F0);

	float NDF = DistributionGGX(N, H, props.roughness);
	float G = GeometrySmith(N, V, L, props.roughness);

	vec3 numerator = NDF * G * F;
	float denomirator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denomirator, 0.001);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - props.metallic;
	float shadow = 0.0;
	
    //if (light.castShadows) {
		//float bias = max(0.05 * (1.0 - dot(N, lightDir)), 0.005);
		//bias = 0.0002;
		//shadow = PCSSshadows(fragPosLightSpaceDirLight, 0.05, light.shadowMap, light.size);
	//}

	float NdotL = max(dot(N, L), 0.0);
	return (kD * props.albedo / PI + specular) * radiance * NdotL*(1-shadow);
}

SurfaceProperties getSurfaceProperties(){
    SurfaceProperties properties;
    if (material.hasAlbedo){
        properties.albedo = pow(texture(material.albedoMap, texCoords).rgb, vec3(2.2));
		//properties.albedo = vec3(1.0f, 0.0f, 0.0f);
	}
    else {
        properties.albedo = material.albedo;
    }

    properties.roughness = material.roughness;
    properties.metallic = material.metallic;
    properties.ao = material.ao;

    return properties;
}

void main(){

    SurfaceProperties props = getSurfaceProperties();

    vec3 V = normalize(viewPos-fragPos);
    vec3 N = normalize(normal);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, props.albedo, props.metallic);
	vec3 L0 = vec3(0.0);

    vec3 ambient = 0.05f*props.albedo*material.ao;
    
    vec3 color = ambient;
    color += CalcDirectionalLight(dirlight, N, V, F0, props);

    //tone mapping
    const float gamma = 2.2;
    vec3 mapped = vec3(1.0)-exp(-color*exposure);
    mapped = pow(mapped, vec3(1.0/gamma));

    //vec3 texColor = vec3(texture(Tex, texCoords));
    FragColor = vec4(mapped,  1.0f);
}