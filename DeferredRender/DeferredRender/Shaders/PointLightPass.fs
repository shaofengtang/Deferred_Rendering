#version 330 core

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColorSpec;

uniform vec3 viewPos;
uniform float shininess;
uniform vec2 screenSize;

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

uniform PointLight pointLight;

vec2 CalcTexCoord()
{
	float x = gl_FragCoord.x / screenSize.x;
	float y = gl_FragCoord.y / screenSize.y;
	
	return vec2(x, y);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 diffuseColor, float specularScale, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
	float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	
	vec3 ambient = light.ambient * diffuseColor;
	vec3 diffuse = light.diffuse * diff * diffuseColor;
	vec3 specular = light.specular * spec * vec3(specularScale, specularScale, specularScale);
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

void main()
{
	vec2 TexCoords = CalcTexCoord();
	vec3 pos = texture(gPosition, TexCoords).rgb;
	vec3 norm = texture(gNormal, TexCoords).rgb;
	vec3 diffuse = texture(gColorSpec, TexCoords).rgb;
	float spec = texture(gColorSpec, TexCoords).a;
	
	vec3 viewDir = normalize(viewPos - pos);
	
	vec3 color = vec3(0.0, 0.0, 0.0);
	color += CalcPointLight(pointLight, norm, pos, diffuse, spec, viewDir);
	
	FragColor = vec4(color, 1.0);
	
}