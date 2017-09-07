#include <iostream>
#include <vector>
#include <map>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "Camera.h"
#include "stb_image.h"

#include "mesh.h"
#include "model.h"
#include "light.h"
#include "TextRender.h"


float deltaTime = 0.0f;
float lastFrame = 0.0f;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

Camera camera(glm::vec3(-30.0f, 10.0f, 0.0f));

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void RenderWithDepth(Shader& depthShader, Model& model, float n, float f)
{
	depthShader.use();
	depthShader.setFloat("near", n);
	depthShader.setFloat("far", f);
	model.Draw(depthShader);
}

int main()
{
	std::cout << "Deferred Renderred Demo:\n";

	std::cout << "Initializing Window and OpenGL...\n";
	// init flfw
	glfwInit();

	// set up opengl context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create main window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Deferred Renderer", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// init GLAD, load function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// register callback functions
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// model loading
	std::cout << "Loading models...\n";
	Model building("Models//Sponza//fixed_sponza.fbx");

	std::cout << "Loading shaders...\n";
	// shader configuration
	Shader texShader("Shaders//VertexShaderWithTex.vs", "Shaders//FragmentShaderWithTex.fs");
	Shader ndcShader("Shaders//NDC.vs", "Shaders//NDC.fs");
	Shader textShader("Shaders//Char2D.vs", "Shaders//Char2D.fs");
	//Shader lampShader("Shaders//lampShader.vs", "Shaders//lampShader.fs");
	Shader geoShader("Shaders//GeometryPass.vs", "Shaders//GeometryPass.fs");
	Shader lightShader("Shaders//LightPass.vs", "Shaders//LightPass.fs");
	Shader pointlightShader("Shaders//LightVolume.vs", "Shaders//PointLightPass.fs");
	Shader nullShader("Shaders//Null.vs", "Shaders//Null.fs");

	std::cout << "Initializing text renderer...\n";
	// text renderer
	TextRender textRender;
	textRender.LoadFont("Fonts//arial.ttf");

	std::cout << "Generating lights...";
	// setup lights
	//DirLight dirLight(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.2f, 0.2f, 0.2f));
	//dirLight.SetupInShader(lightShader, "dirLight");

	std::vector<PointLight> pointLights;
	FILE *f;
	fopen_s(&f, "Lights//lights.data", "r");
	if (f)
	{
		char buffer[512];
		while (fgets(buffer, 512, f))
		{
			float px, py, pz;
			float c, l, q;
			float ar, ag, ab, dr, dg, db, sr, sg, sb;
			sscanf_s(buffer, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
				&px, &py, &pz, &c, &l, &q, &ar, &ag, &ab, &dr, &dg, &db, &sr, &sg, &sb);
			pointLights.push_back(PointLight(glm::vec3(px, py, pz), c, l, q, glm::vec3(ar, ag, ab), glm::vec3(dr, dg, db), glm::vec3(sr, sg, sb)));
		}
		fclose(f);
	}
	else
	{
		std::cout << "Failed to generate lights. The progream exits.\n";
		exit(0);
	}
	for (int i = 0; i < pointLights.size(); ++i)
	{
		std::stringstream ss;
		ss << i;
		pointLights[i].SetupInShader(lightShader, (std::string("pointLights[") + ss.str() + std::string("]")).c_str());
		//pointLights[i].SetupInShader(texShader, (std::string("pointLights[") + ss.str() + std::string("]")).c_str());
	}

	std::cout << "Setup shaders' uniforms...\n";
	// Setup uniforms
	lightShader.use();
	lightShader.setFloat("shininess", 4.0f);

	texShader.use();
	texShader.setFloat("shininess", 4.0f);

	pointlightShader.use();
	pointlightShader.setFloat("shininess", 4.0f);
	pointlightShader.setVec2("screenSize", (float)SCR_WIDTH, (float)SCR_HEIGHT);

	//
	float quad[] =
	{
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	/*float quads[4][24] =
	{
		// top-left
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, 0.0f,  0.0f, 0.0f,
		0.0f, 0.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		0.0f, 0.0f,  1.0f, 0.0f,
		0.0f,  1.0f,  1.0f, 1.0f,
		// top-right
		0.0f,  1.0f,  0.0f, 1.0f,
		0.0f, 0.0f,  0.0f, 0.0f,
		1.0f, 0.0f,  1.0f, 0.0f,

		0.0f,  1.0f,  0.0f, 1.0f,
		1.0f, 0.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f,
		// bottom-left
		- 1.0f,  0.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		0.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  0.0f,  0.0f, 1.0f,
		0.0f, -1.0f,  1.0f, 0.0f,
		0.0f,  0.0f,  1.0f, 1.0f,
		// bottom-right
		0.0f,  0.0f,  0.0f, 1.0f,
		0.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		0.0f,  0.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  0.0f,  1.0f, 1.0f
	};*/

	unsigned int screenVAO;
	glGenVertexArrays(1, &screenVAO);
	glBindVertexArray(screenVAO);

	unsigned int screenVBO;
	glGenBuffers(1, &screenVBO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	/*unsigned int screenVAOs[4];
	glGenVertexArrays(4, screenVAOs);
	unsigned int screenVBOs[4];
	glGenBuffers(4, screenVBOs);

	for (int i = 0; i < 4; ++i)
	{
		glBindVertexArray(screenVAOs[i]);
		glBindBuffer(GL_ARRAY_BUFFER, screenVBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quads[i]), quads[i], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}*/

	std::cout << "Initializing G-Buffers...\n";
	// create and bind FBO
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	unsigned int gPosition, gNormal, gColorSpec;
	// position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	// attach it to currently bound FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// normal buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// attach it to currently bound FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// color + specular intensity buffer
	glGenTextures(1, &gColorSpec);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// attach it to currently bound FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

	// tell openGl which color attachment we'll use for rendering
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	// depth + stencil
	unsigned int gDepthStencil;
	glGenTextures(1, &gDepthStencil);
	glBindTexture(GL_TEXTURE_2D, gDepthStencil);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// attach it to currently bound FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, gDepthStencil, 0);

	// intermediate
	unsigned int gFinal;
	glGenTextures(1, &gFinal);
	glBindTexture(GL_TEXTURE_2D, gFinal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// attach it to currently bound FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gFinal, 0);


	// generate renderbuffer (as depth buffer and stencil buffer)
	/*unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach it to currently bound FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);*/

	// check framebuffer status
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		// model, view, projection matrix setup
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model;

		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 400.0f);
		view = camera.GetViewMatrix();
		model = glm::scale(model, glm::vec3(0.05f));

		// setup some lights and uniform
		geoShader.use();
		geoShader.setMat4("projection", glm::value_ptr(projection));
		geoShader.setMat4("view", glm::value_ptr(view));
		geoShader.setMat4("model", glm::value_ptr(model));

		lightShader.use();
		lightShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);

		texShader.use();
		texShader.setMat4("projection", glm::value_ptr(projection));
		texShader.setMat4("view", glm::value_ptr(view));
		texShader.setMat4("model", glm::value_ptr(model));
		texShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);

		pointlightShader.use();
		pointlightShader.setMat4("projection", glm::value_ptr(projection));
		pointlightShader.setMat4("view", glm::value_ptr(view));
		pointlightShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);

		nullShader.use();
		nullShader.setMat4("projection", glm::value_ptr(projection));
		nullShader.setMat4("view", glm::value_ptr(view));

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT3);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// #1 Geometry Pass
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glDrawBuffers(3, attachments);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		
		//glDisable(GL_CULL_FACE);
		
		//glDisable(GL_BLEND);

		geoShader.use();
		building.Draw(geoShader);

		glDepthMask(GL_FALSE);

		// Show gBuffers
		/*glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		ndcShader.use();
		glBindVertexArray(screenVAOs[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(screenVAOs[1]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(screenVAOs[2]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gColorSpec);
		glDrawArrays(GL_TRIANGLES, 0, 6);*/

		// #2 Lighting Pass
		/*lightShader.use();
		lightShader.setInt("gPosition", 0);
		lightShader.setInt("gNormal", 1);
		lightShader.setInt("gColorSpec", 2);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(screenVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gColorSpec);

		glDrawArrays(GL_TRIANGLES, 0, 6);*/

		// #2 Improved Lighting Pass with Light Volume
		/*pointlightShader.use();
		pointlightShader.setInt("gPosition", 0);
		pointlightShader.setInt("gNormal", 1);
		pointlightShader.setInt("gColorSpec", 2);

		// we enable here because we render the scene for each point light
		// and finally add them up
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gColorSpec);

		for (int i = 0; i < pointLights.size(); ++i)
		{
			//glClear(GL_DEPTH_BUFFER_BIT);
			pointLights[i].SetupInShader(pointlightShader, "pointLight");
			pointLights[i].RenderLightVolume(pointlightShader);
		}

		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);*/

		// #2 Advanced light pass with stencil pass
		// #2.1 Stencil Pass
		
		glEnable(GL_STENCIL_TEST);

		for (int i = 0; i < pointLights.size(); ++i)
		{
			// stencil pass
			nullShader.use();
			//glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glDrawBuffer(GL_NONE);
			
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glClearStencil(0);
			glStencilMask(0xFF);
			glClear(GL_STENCIL_BUFFER_BIT);

			glStencilFunc(GL_ALWAYS, 0, 0);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

			pointLights[i].RenderLightVolume(nullShader);

			// light pass
			glDrawBuffer(GL_COLOR_ATTACHMENT3);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gColorSpec);

			pointlightShader.use();
			pointlightShader.setInt("gPosition", 0);
			pointlightShader.setInt("gNormal", 1);
			pointlightShader.setInt("gColorSpec", 2);

			glStencilMask(0x00);
			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			
			pointLights[i].SetupInShader(pointlightShader, "pointLight");
			pointLights[i].RenderLightVolume(pointlightShader);

			glCullFace(GL_BACK);
			glDisable(GL_BLEND);

		}

		glDisable(GL_STENCIL_TEST);

		// #3 final pass
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glReadBuffer(GL_COLOR_ATTACHMENT3);
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);


		// Forward rendering
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		/*glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		texShader.use();
		building.Draw(texShader);*/

		// text rendering
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		std::string strFPS = "FPS: ";
		std::stringstream ss;
		float fps = 1.0f / deltaTime;
		ss << fps;
		textRender.RenderText(textShader, (strFPS + ss.str()).c_str(), 5.0f, 580.0f, 0.4f);
		glDisable(GL_BLEND);

		// check and call evnets and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);

	// clean up
	glfwTerminate();
	return 0;
}