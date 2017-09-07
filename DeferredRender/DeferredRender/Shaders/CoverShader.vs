#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord; // This should be (0.0, 0.0) for all vertices

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec3 extPos = aPos + aNormal * 0.05;
	gl_Position = projection * view * model * vec4(extPos, 1.0f);
}
