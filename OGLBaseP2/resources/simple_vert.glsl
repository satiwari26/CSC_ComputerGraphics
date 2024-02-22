#version 330 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec3 lightPos;
uniform vec3 lightPos2; // Position of the second light

out vec3 fragNor;
out vec3 lightDir;
out vec3 lightDir2; // Direction to the second light
out vec3 EPos;

void main()
{
    gl_Position = P * V * M * vertPos;

    fragNor = (V * M * vec4(vertNor, 0.0)).xyz; 
    lightDir = vec3(V * vec4(lightPos,1)) - (V * M*vertPos).xyz;
    lightDir2 = vec3(V * vec4(lightPos2,1)) - (V * M*vertPos).xyz; // Calculate the direction to the second light
    EPos = (V*M*vertPos).xyz;
}