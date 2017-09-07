#version 330 core

out vec4 FragColor;

uniform float near;
uniform float far;

float LinearizeDepth(float depth)
{
	// Original equation: depth = (1/z - 1/near) / (1/far - 1/near)
	return (far * near) / ((near - far) * depth + far);
};

void main()
{
	float depth = LinearizeDepth(gl_FragCoord.z) / far;
	FragColor = vec4(vec3(depth), 1.0);
}