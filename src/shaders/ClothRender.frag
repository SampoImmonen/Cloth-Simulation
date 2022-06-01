#version 430

const float PI = 3.14159265359;
const int num_pcf_samples = 64;
const int num_blocker_samples = 16;

const vec2 poissonDisk[64] = vec2[](
	vec2(-0.04117257f, -0.1597612f),
	vec2(0.06731031f, -0.4353096f),
	vec2(-0.206701f, -0.4089882f),
	vec2(0.1857469f, -0.2327659f),
	vec2(-0.2757695f, -0.159873f),
	vec2(-0.2301117f, 0.1232693f),
	vec2(0.05028719f, 0.1034883f),
	vec2(0.236303f, 0.03379251f),
	vec2(0.1467563f, 0.364028f),
	vec2(0.516759f, 0.2052845f),
	vec2(0.2962668f, 0.2430771f),
	vec2(0.3650614f, -0.1689287f),
	vec2(0.5764466f, -0.07092822f),
	vec2(-0.5563748f, -0.4662297f),
	vec2(-0.3765517f, -0.5552908f),
	vec2(-0.4642121f, -0.157941f),
	vec2(-0.2322291f, -0.7013807f),
	vec2(-0.05415121f, -0.6379291f),
	vec2(-0.7140947f, -0.6341782f),
	vec2(-0.4819134f, -0.7250231f),
	vec2(-0.7627537f, -0.3445934f),
	vec2(-0.7032605f, -0.13733f),
	vec2(0.8593938f, 0.3171682f),
	vec2(0.5223953f, 0.5575764f),
	vec2(0.7710021f, 0.1543127f),
	vec2(0.6919019f, 0.4536686f),
	vec2(0.3192437f, 0.4512939f),
	vec2(0.1861187f, 0.595188f),
	vec2(0.6516209f, -0.3997115f),
	vec2(0.8065675f, -0.1330092f),
	vec2(0.3163648f, 0.7357415f),
	vec2(0.5485036f, 0.8288581f),
	vec2(-0.2023022f, -0.9551743f),
	vec2(0.165668f, -0.6428169f),
	vec2(0.2866438f, -0.5012833f),
	vec2(-0.5582264f, 0.2904861f),
	vec2(-0.2522391f, 0.401359f),
	vec2(-0.428396f, 0.1072979f),
	vec2(-0.06261792f, 0.3012581f),
	vec2(0.08908027f, -0.8632499f),
	vec2(0.9636437f, 0.05915006f),
	vec2(0.8639213f, -0.309005f),
	vec2(-0.03422072f, 0.6843638f),
	vec2(-0.3734946f, -0.8823979f),
	vec2(-0.3939881f, 0.6955767f),
	vec2(-0.4499089f, 0.4563405f),
	vec2(0.07500362f, 0.9114207f),
	vec2(-0.9658601f, -0.1423837f),
	vec2(-0.7199838f, 0.4981934f),
	vec2(-0.8982374f, 0.2422346f),
	vec2(-0.8048639f, 0.01885651f),
	vec2(-0.8975322f, 0.4377489f),
	vec2(-0.7135055f, 0.1895568f),
	vec2(0.4507209f, -0.3764598f),
	vec2(-0.395958f, -0.3309633f),
	vec2(-0.6084799f, 0.02532744f),
	vec2(-0.2037191f, 0.5817568f),
	vec2(0.4493394f, -0.6441184f),
	vec2(0.3147424f, -0.7852007f),
	vec2(-0.5738106f, 0.6372389f),
	vec2(0.5161195f, -0.8321754f),
	vec2(0.6553722f, -0.6201068f),
	vec2(-0.2554315f, 0.8326268f),
	vec2(-0.5080366f, 0.8539945f)
	);

out vec4 FragColor;

in vec3 normal;
in vec2 texCoords;
in vec3 fragPos;
in vec4 fragPosLightSpace;
in mat3 TBN;

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

	bool hasNormal;
	sampler2D normalMap;

	bool hasMetallic;
	sampler2D metallicMap;

	bool hasRoughness;
	sampler2D roughnessMap;
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


vec2 blockerSearchDirectionalLight(vec2 texCoords, float zReceiver, float bias, sampler2D shadowMap, float size) {
	float blockers_dist = 0;
	float num_blockers = 0;
	for (int i = 0; i < num_blocker_samples; i++) {
		vec2 stexCoords = texCoords + poissonDisk[i] * size;
		float b_dist = texture(shadowMap, stexCoords).r;
		if (b_dist < zReceiver - bias) {
			num_blockers++;
			blockers_dist += b_dist;
		}
	}
	return vec2(blockers_dist / num_blockers, num_blockers);
}


float penumbraSize(float zReceiver, float avgBlockerDist) {
	return (zReceiver - avgBlockerDist) / zReceiver;
}

float pcf(vec2 texCoords, float zReceiver, float filterRadius, float bias, sampler2D shadowMap) {
	float sum = 0.0;
	for (int i = 0; i < num_pcf_samples; i++) {
		vec2 sTexCoords = texCoords + poissonDisk[i] * filterRadius;
		if (texture(shadowMap, sTexCoords).r < zReceiver - bias) {
			sum += 1.0;
		}
	}
	return sum / num_pcf_samples;
}

float PCSSshadows(vec4 fragPosLightSpace, float bias, sampler2D shadowMap, float size) {
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	vec2 ptexCoords = projCoords.xy;
	vec2 blocker_stats = blockerSearchDirectionalLight(ptexCoords, projCoords.z, bias, shadowMap, size);
	if (blocker_stats.y < 1) {
		return 0.0f;
	}
	float filterradius = size * penumbraSize(projCoords.z, blocker_stats[0]);
	return pcf(ptexCoords, projCoords.z, filterradius, bias, shadowMap);
}

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
	
    if (light.shadows) {
		float bias = max(0.05 * (1.0 - dot(normal, L)), 0.0);
		//bias = 0.0002;
		shadow = PCSSshadows(fragPosLightSpace, bias, light.shadowMap, light.size);
	}

	float NdotL = max(dot(normal, L), 0.0);
	return (kD * props.albedo / PI + specular) * radiance*NdotL*(1-shadow);
}

SurfaceProperties getSurfaceProperties(){
    SurfaceProperties properties;
    if (material.hasAlbedo){
        properties.albedo = pow(texture(material.albedoMap, texCoords).rgb, vec3(2.2));
	}
    else {
        properties.albedo = material.albedo;
    }
	if (material.hasMetallic){
        //properties.metallic = pow(texture(material.metallicMap, texCoords).rgb, vec3(2.2));
	}
    else {
        properties.metallic = material.metallic;
    }
	if (material.hasRoughness){
        //properties.roughness = pow(texture(material.roughnessMap, texCoords).rgb, vec3(2.2));
	}
    else {
        properties.roughness = material.roughness;
    }

    properties.ao = material.ao;

    return properties;
}

void main(){

    SurfaceProperties props = getSurfaceProperties();

	vec3 N = normalize(normal);
	if (material.hasNormal){
		N = texture(material.normalMap, texCoords).rgb;
		N = normalize(N*2.0-1.0);
		N = normalize(TBN*N);
	}

    vec3 V = normalize(viewPos-fragPos);
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