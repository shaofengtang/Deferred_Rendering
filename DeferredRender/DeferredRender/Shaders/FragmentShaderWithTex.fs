#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform float shininess;

struct DirLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;
	float constant;
	float linear;
	float quadratic;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	float constant;
	float linear;
	float quadratic;
	float innerCutoff;
	float outerCutoff;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform DirLight dirLight;
const int NR_POINT_LIGHTS = 500;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform vec3 viewPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	
	float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	
	vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoord));
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
	float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	
	vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoord));
	//vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoord));
	float specularScale = texture(texture_specular1, TexCoord).r;
	vec3 specular = light.specular * spec * vec3(specularScale, specularScale, specularScale);
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.innerCutoff - light.outerCutoff;
	float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
	
	float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	
	vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoord));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoord));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	diffuse *= intensity;
	specular *= intensity;
	return (ambient + diffuse + specular);
}

void main()

{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	
	vec3 result = vec3(0.0, 0.0, 0.0);
	
	for (int i = 0; i < NR_POINT_LIGHTS; ++i)
	{
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	}
	
	
	FragColor = vec4(result, 1.0);
}