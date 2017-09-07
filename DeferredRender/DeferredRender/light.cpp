#include <string>

#include "light.h"
#include "shader.h"

#define PI 3.1415926

/* Light Class Implamentation */
float m_vertices[108] =
{
	// Back face
	-0.5f, -0.5f, -0.5f, // Bottom-left
	0.5f, 0.5f, -0.5f, // top-right
	0.5f, -0.5f, -0.5f, // bottom-right
	0.5f, 0.5f, -0.5f, // top-right
	-0.5f, -0.5f, -0.5f, // bottom-left
	-0.5f, 0.5f, -0.5f, // top-left
	// Front face
	-0.5f, -0.5f, 0.5f, // bottom-left
	0.5f, -0.5f, 0.5f, // bottom-right
	0.5f, 0.5f, 0.5f, // top-right
	0.5f, 0.5f, 0.5f, // top-right
	-0.5f, 0.5f, 0.5f, // top-left
	-0.5f, -0.5f, 0.5f, // bottom-left
	// Left face
	-0.5f, 0.5f, 0.5f, // top-right
	-0.5f, 0.5f, -0.5f, // top-left
	-0.5f, -0.5f, -0.5f, // bottom-left
	-0.5f, -0.5f, -0.5f, // bottom-left
	-0.5f, -0.5f, 0.5f, // bottom-right
	-0.5f, 0.5f, 0.5f, // top-right
	// Right face
	0.5f, 0.5f, 0.5f, // top-left
	0.5f, -0.5f, -0.5f, // bottom-right
	0.5f, 0.5f, -0.5f, // top-right         
	0.5f, -0.5f, -0.5f, // bottom-right
	0.5f, 0.5f, 0.5f, // top-left
	0.5f, -0.5f, 0.5f, // bottom-left     
	// Bottom face
	-0.5f, -0.5f, -0.5f, // top-right
	0.5f, -0.5f, -0.5f, // top-left																															   
	0.5f, -0.5f, 0.5f, // bottom-left											   
	0.5f, -0.5f, 0.5f, // bottom-left																											   
	-0.5f, -0.5f, 0.5f, // bottom-right																									   
	-0.5f, -0.5f, -0.5f, // top-right
	// Top face																														 
	-0.5f, 0.5f, -0.5f, // top-left																																									 
	0.5f, 0.5f, 0.5f, // bottom-right																																									 
	0.5f, 0.5f, -0.5f, // top-right     																																									
	0.5f, 0.5f, 0.5f, // bottom-right																																								 
	-0.5f, 0.5f, -0.5f, // top-left																																								 
	-0.5f, 0.5f, 0.5f, // bottom-left        
};

Light::Light(glm::vec3 ambient/* = glm::vec3(0.1f, 0.1f, 0.1f)*/, glm::vec3 diffuse/* = glm::vec3(0.8f, 0.8f, 0.8f)*/, glm::vec3 specular/* = glm::vec3(1.0f, 1.0f, 1.0f)*/)
{
	m_ambient = ambient;
	m_diffuse = diffuse;
	m_specular = specular;
}

void Light::SetAmbient(float x, float y, float z)
{
	m_ambient.x = x;
	m_ambient.y = y;
	m_ambient.z = z;
}

void Light::SetDiffuse(float x, float y, float z)
{
	m_diffuse.x = x;
	m_diffuse.y = y;
	m_diffuse.z = z;
}

void Light::SetSpecular(float x, float y, float z)
{
	m_specular.x = x;
	m_specular.y = y;
	m_specular.z = z;
}

/* DirLight Class Implementation */
DirLight::DirLight(glm::vec3 direction/* = glm::vec3(0.0f, 0.0f, -1.0f)*/, glm::vec3 ambient/* = glm::vec3(0.1f, 0.1f, 0.1f)*/, glm::vec3 diffuse/* = glm::vec3(0.8f, 0.8f, 0.8f)*/, glm::vec3 specular/* = glm::vec3(1.0f, 1.0f, 1.0f)*/)
	: Light(ambient, diffuse, specular)
{
	m_direction = direction;
}

void DirLight::SetDirection(float x, float y, float z)
{
	m_direction.x = x;
	m_direction.y = y;
	m_direction.z = z;
}

void DirLight::SetupInShader(Shader& shader, const char* uniName)
{
	shader.use();
	std::string name(uniName);
	shader.setVec3(name + ".direction", m_direction.x, m_direction.y, m_direction.z);
	shader.setVec3(name + ".ambient", m_ambient.x, m_ambient.y, m_ambient.z);
	shader.setVec3(name + ".diffuse", m_diffuse.x, m_diffuse.y, m_diffuse.z);
	shader.setVec3(name + ".specular", m_specular.x, m_specular.y, m_specular.z);
}

void DirLight::RenderLight(Shader& shader)
{
	// do nothing, no visual representation for the dirlights
}

/* PointLight Class Implementation */
PointLight::PointLight(glm::vec3 position, float constant/* = 1.0f*/, float linear/* = 0.09f*/, float quadratic/* = 0.0032f*/, glm::vec3 ambient/* = glm::vec3(0.1f, 0.1f, 0.1f)*/, glm::vec3 diffuse/* = glm::vec3(0.8f, 0.8f, 0.8f)*/, glm::vec3 specular/* = glm::vec3(1.0f, 1.0f, 1.0f)*/)
	: Light(ambient, diffuse, specular)
{
	m_position = position;
	m_constant = constant;
	m_linear = linear;
	m_quadratic = quadratic;

	m_model = glm::translate(m_model, m_position);
	setupRepresentation();

	generateLightVolume(16, 4);
	calcRadiusAndMat();
	setupLightVolumeRender();
}

void PointLight::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;

	m_model = glm::translate(m_model, m_position);
	calcRadiusAndMat();
}

void PointLight::SetConstant(float constant)
{
	m_constant = constant;
	calcRadiusAndMat();
}

void PointLight::SetLinear(float linear)
{
	m_linear = linear;
	calcRadiusAndMat();
}

void PointLight::SetQuadratic(float quadratic)
{
	m_quadratic = quadratic;
	calcRadiusAndMat();
}

void PointLight::SetupInShader(Shader& shader, const char* uniName)
{
	shader.use();
	std::string name(uniName);
	shader.setVec3(name + ".position", m_position.x, m_position.y, m_position.z);
	shader.setVec3(name + ".ambient", m_ambient.x, m_ambient.y, m_ambient.z);
	shader.setVec3(name + ".diffuse", m_diffuse.x, m_diffuse.y, m_diffuse.z);
	shader.setVec3(name + ".specular", m_specular.x, m_specular.y, m_specular.z);
	shader.setFloat(name + ".constant", m_constant);
	shader.setFloat(name + ".linear", m_linear);
	shader.setFloat(name + ".quadratic", m_quadratic);
}

void PointLight::setupRepresentation()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	
	glBindVertexArray(0);
}

void PointLight::RenderLight(Shader& shader)
{
	shader.use();
	shader.setMat4("model", glm::value_ptr(m_model));
	shader.setVec3("lightColor", m_diffuse.x, m_diffuse.y, m_diffuse.z);

	// draw representation
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void PointLight::RenderLightVolume(Shader& shader)
{
	shader.use();
	shader.setMat4("model", glm::value_ptr(m_volumeModel));

	glBindVertexArray(m_volumeVAO);
	glDrawElements(GL_TRIANGLES, m_sphereIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void PointLight::generateLightVolume(int slices, int stack)
{
	GLfloat angle = 2 * PI / slices;
	GLfloat theta = PI / 2.0 / stack;

	m_sphereVertices.resize((slices * (2 * stack - 1) + 2) * 3);
	
	// the top vertex
	m_sphereVertices[0] = 0.0;
	m_sphereVertices[1] = 1.0;
	m_sphereVertices[2] = 0.0;
	// the bottom vertex
	m_sphereVertices[(slices * (2 * stack - 1) + 1) * 3] = 0.0;
	m_sphereVertices[(slices * (2 * stack - 1) + 1) * 3 + 1] = -1.0;
	m_sphereVertices[(slices * (2 * stack - 1) + 1) * 3 + 2] = 0.0;
	// generate remaining vertices
	for (int i = 0; i < stack; i++)
	{
		for (int j = 0; j < slices; j++)
		{
			m_sphereVertices[3 + (i * slices + j) * 3] = sin(theta * (i + 1)) * sin(angle * j);
			m_sphereVertices[3 + (i * slices + j) * 3 + 1] = cos(theta * (i + 1));
			m_sphereVertices[3 + (i * slices + j) * 3 + 2] = sin(theta * (i + 1)) * cos(angle * j);
		}
	}
	for (int i = 0; i < stack - 1; i++)
	{
		for (int j = 0; j < slices; j++)
		{
			m_sphereVertices[((stack + i) * slices + 1 + j) * 3] = cos(theta * (i + 1)) * sin(angle * j);
			m_sphereVertices[((stack + i) * slices + 1 + j) * 3 + 1] = -sin(theta * (i + 1));
			m_sphereVertices[((stack + i) * slices + 1 + j) * 3 + 2] = cos(theta * (i + 1)) * cos(angle * j);
		}
	}
	// generate indices
	for (int i = 1; i < slices; ++i)
	{
		m_sphereIndices.push_back(0);
		m_sphereIndices.push_back(i);
		m_sphereIndices.push_back(i + 1);
	}
	m_sphereIndices.push_back(0);
	m_sphereIndices.push_back(slices);
	m_sphereIndices.push_back(1);
	for (int i = 0; i < (stack - 1) * 2; ++i)
	{
		for (int j = 1; j < slices; j++)
		{
			m_sphereIndices.push_back(i * slices + j);
			m_sphereIndices.push_back((i + 1) * slices + j);
			m_sphereIndices.push_back((i + 1) * slices + j + 1);
			m_sphereIndices.push_back(i * slices + j);
			m_sphereIndices.push_back((i + 1) * slices + j + 1);
			m_sphereIndices.push_back(i * slices + j + 1);
		}
		m_sphereIndices.push_back(i * slices + slices);
		m_sphereIndices.push_back((i + 1) * slices + slices);
		m_sphereIndices.push_back((i + 1) * slices + 1);
		m_sphereIndices.push_back(i * slices + slices);
		m_sphereIndices.push_back((i + 1) * slices + 1);
		m_sphereIndices.push_back(i * slices + 1);
	}
	int size = m_sphereVertices.size() / 3;
	for (int i = 0; i < slices - 1; ++i)
	{
		m_sphereIndices.push_back(size - 1 - slices + i);
		m_sphereIndices.push_back(size - 1);
		m_sphereIndices.push_back(size - 1 - slices + i + 1);
	}
	m_sphereIndices.push_back(size - 2);
	m_sphereIndices.push_back(size - 1);
	m_sphereIndices.push_back(size - 1 - slices);

	// debug
	/*FILE *f;
	fopen_s(&f, "sphere.obj", "w");
	if (f)
	{
		for (int i = 0; i < m_sphereVertices.size(); i = i + 3)
		{
			fprintf(f, "v %f %f %f\n", m_sphereVertices[i], m_sphereVertices[i + 1], m_sphereVertices[i + 2]);
		}
		for (int i = 0; i < m_sphereVertices.size(); i = i + 3)
		{
			fprintf(f, "vn %f %f %f\n", m_sphereVertices[i], m_sphereVertices[i + 1], m_sphereVertices[i + 2]);
		}
		for (int i = 0; i < m_sphereIndices.size(); i = i + 3)
		{
			fprintf(f, "f %d//%d %d//%d %d//%d\n", m_sphereIndices[i] + 1, m_sphereIndices[i] + 1, m_sphereIndices[i + 1] + 1, m_sphereIndices[i + 1] + 1, m_sphereIndices[i + 2] + 1, m_sphereIndices[i + 2] + 1);
		}
		fclose(f);
	}
	else
	{
		std::cout << "Cannot open file.\n";
	}*/
}

void PointLight::calcRadiusAndMat()
{
	float maxIntensity = 0.0f;
	if (m_diffuse.r >= m_diffuse.g)
	{
		if (m_diffuse.r >= m_diffuse.b)
			maxIntensity = m_diffuse.r;
		else
			maxIntensity = m_diffuse.b;
	}
	else
	{
		if (m_diffuse.g >= m_diffuse.b)
			maxIntensity = m_diffuse.g;
		else
			maxIntensity = m_diffuse.b;
	}

	// we set throuheld to 1 / 256 in light attenuation model
	// we get 1 / 256 = intensity * 1 / (c + d * l + d * d * q)
	m_radius = (-m_linear + std::sqrtf(m_linear * m_linear - 4 * m_quadratic * (m_constant - 256 * maxIntensity))) / (2 * m_quadratic);
	
	m_volumeModel = glm::translate(m_volumeModel, m_position);
	m_volumeModel = glm::scale(m_volumeModel, glm::vec3(m_radius, m_radius, m_radius));
	
}

void PointLight::setupLightVolumeRender()
{
	glGenVertexArrays(1, &m_volumeVAO);
	glGenBuffers(1, &m_volumeVBO);
	glGenBuffers(1, &m_volumeEBO);

	glBindVertexArray(m_volumeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_volumeVBO);
	glBufferData(GL_ARRAY_BUFFER, m_sphereVertices.size() * sizeof(float), &m_sphereVertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_volumeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_sphereIndices.size() * sizeof(unsigned int), &m_sphereIndices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
}

/* SpotLight Class Implementation */
SpotLight::SpotLight(glm::vec3 position, glm::vec3 direction, float constant/* = 1.0f*/, float linear/* = 0.09f*/, float quadratic/* = 0.0032f*/, float innerCutoff/* = 0.218166f*//*12.5 deg*/, float outerCutoff/* = 0.305433f*//*17.5 deg*/, glm::vec3 ambient/* = glm::vec3(0.1f, 0.1f, 0.1f)*/, glm::vec3 diffuse/* = glm::vec3(0.8f, 0.8f, 0.8f)*/, glm::vec3 specular/* = glm::vec3(1.0f, 1.0f, 1.0f)*/)
	: Light(ambient, diffuse, specular)
{
	m_position = position;
	m_direction = direction;
	m_constant = constant;
	m_linear = linear;
	m_quadratic = quadratic;
	m_innerCutoff = innerCutoff;
	m_outerCutoff = outerCutoff;

	m_model = glm::translate(m_model, m_position);
	setupRepresentation();
}

void SpotLight::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;

	m_model = glm::translate(m_model, m_position);
}

void SpotLight::SetDirection(float x, float y, float z)
{
	m_direction.x = x;
	m_direction.y = y;
	m_direction.z = z;
}

void SpotLight::SetConstant(float constant)
{
	m_constant = constant;
}

void SpotLight::SetLinear(float linear)
{
	m_linear = linear;
}

void SpotLight::SetQuadratic(float quadratic)
{
	m_quadratic = quadratic;
}

void SpotLight::SetInnerCutoff(float innerCutoff)
{
	m_innerCutoff = innerCutoff;
}

void SpotLight::SetOuterCutoff(float outerCutoff)
{
	m_outerCutoff = outerCutoff;
}

void SpotLight::SetupInShader(Shader& shader, const char* uniName)
{
	shader.use();
	std::string name(uniName);
	shader.setVec3(name + ".position", m_position.x, m_position.y, m_position.z);
	shader.setVec3(name + ".direction", m_direction.x, m_direction.y, m_direction.z);
	shader.setVec3(name + ".ambient", m_ambient.x, m_ambient.y, m_ambient.z);
	shader.setVec3(name + ".diffuse", m_diffuse.x, m_diffuse.y, m_diffuse.z);
	shader.setVec3(name + ".specular", m_specular.x, m_specular.y, m_specular.z);
	shader.setFloat(name + ".constant", m_constant);
	shader.setFloat(name + ".linear", m_linear);
	shader.setFloat(name + ".quadratic", m_quadratic);
	shader.setFloat(name + ".innerCutoff", m_innerCutoff);
	shader.setFloat(name + ".outerCutoff", m_outerCutoff);
}

void SpotLight::setupRepresentation()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
}

void SpotLight::RenderLight(Shader& shader)
{
	shader.use();
	shader.setMat4("model", glm::value_ptr(m_model));
	shader.setVec3("lightColor", m_diffuse.x, m_diffuse.y, m_diffuse.z);

	// draw representation
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}