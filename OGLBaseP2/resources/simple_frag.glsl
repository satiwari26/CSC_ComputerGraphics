#version 330 core 

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec; // Specular color
uniform float MatShine; // Shininess

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;

void main()
{
	//you will need to work with these for lighting
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
	vec3 viewDir = normalize(-EPos);
	vec3 halfWay = normalize(lightDir + viewDir);

	//calculate specular coefficient
	float sC = pow(max(dot(normal, halfWay), 0.0), MatShine);

	// Calculate the diffuse coefficient, needs to be greater than 0
    float dC = max(dot(normal, light), 0.0);

	//color = vec4(MatAmb*10.0, 1.0);
	// Calculate the final color
    color = vec4(MatAmb + dC * MatDif + sC * MatSpec, 1.0);
}
