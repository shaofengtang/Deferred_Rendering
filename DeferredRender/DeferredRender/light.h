#ifndef __LIGHT_HEADER__
#define __LIGHT_HEADER__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

class Shader;

/* Light Class Definition */
class Light
{
public:
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;

	Light(glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f));
	void SetAmbient(float x, float y, float z);
	void SetDiffuse(float x, float y, float z);
	void SetSpecular(float x, float y, float z);

	virtual void SetupInShader(Shader& shader, const char* uniName) = 0;
	virtual void RenderLight(Shader& shader) = 0;

	static float cubeVertices[108];
																																	 
};

/* DirLight Class Definition */
class DirLight : public Light
{
public:
	glm::vec3 m_direction;

	DirLight(glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f));
	void SetDirection(float x, float y, float z);

	void SetupInShader(Shader& shader, const char* uniName);
	void RenderLight(Shader& shader);
};

/* PointLight Class Definition */
class PointLight : public Light
{
public:
	glm::vec3 m_position;
	// attenuation parameters
	// power = 1.0 / (constant + linear * distance + quadratic * distance ^ 2)
	float m_constant;
	float m_linear;
	float m_quadratic;

	// render representation
	glm::mat4 m_model;

	PointLight(glm::vec3 position, float constant = 1.0f, float linear = 0.09f, float quadratic = 0.0032f, glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f));
	void SetPosition(float x, float y, float z);
	void SetConstant(float constant);
	void SetLinear(float linear);
	void SetQuadratic(float quadratic);

	void SetupInShader(Shader& shader, const char* uniName);
	void RenderLight(Shader& shader);
	void RenderLightVolume(Shader& shader);

private:
	// render stuff
	unsigned int m_VAO;
	unsigned int m_VBO;

	void setupRepresentation();

	// light volume render stuff
	unsigned int m_volumeVAO;
	unsigned int m_volumeVBO;
	unsigned int m_volumeEBO;

	std::vector<float> m_sphereVertices;
	std::vector<unsigned int> m_sphereIndices;
	float m_radius;
	glm::mat4 m_volumeModel;

	void generateLightVolume(int slices, int stack);
	void setupLightVolumeRender();
	void calcRadiusAndMat();
};

/* SpotLight Class Definition */
class SpotLight : public Light
{
public:
	glm::vec3 m_position;
	glm::vec3 m_direction;
	float m_constant;
	float m_linear;
	float m_quadratic;
	float m_innerCutoff;
	float m_outerCutoff;

	// render representation
	glm::mat4 m_model;

	SpotLight(glm::vec3 position, glm::vec3 direction, float constant = 1.0f, float linear = 0.09f, float quadratic = 0.0032f, float innerCutoff = 0.976296f/*cos 12.5 deg*/, float outerCutoff = 0.953717f/*cos 17.5 deg*/, glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f));
	void SetPosition(float x, float y, float z);
	void SetDirection(float x, float y, float z);
	void SetConstant(float constant);
	void SetLinear(float linear);
	void SetQuadratic(float quadratic);
	void SetInnerCutoff(float innerCutoff);
	void SetOuterCutoff(float outerCutoff);

	void SetupInShader(Shader& shader, const char* uniName);
	void RenderLight(Shader& shader);

private:
	// render stuff
	unsigned int m_VAO;
	unsigned int m_VBO;

	void setupRepresentation();
};

#endif
