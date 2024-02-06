#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 verColo;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 fragCol;

void main()
{
	gl_Position = P * V * M * vec4(vertPos, 1.0);
	fragCol = verColo;
}
