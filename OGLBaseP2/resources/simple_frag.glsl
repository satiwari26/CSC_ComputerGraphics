#version 330 core 

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec; // Specular color
uniform float MatShine; // Shininess
uniform float lightIntensity;
uniform float lightIntensity2;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
in vec3 lightDir2; // Light direction for the second light
//position of the vertex in camera space
in vec3 EPos;

void main()
{
    vec3 normal = normalize(fragNor);
    vec3 light = normalize(lightDir);
    vec3 light2 = normalize(lightDir2); // Normalize the direction of the second light
    vec3 viewDir = normalize(-EPos);
    vec3 halfWay = normalize(lightDir + viewDir);
    vec3 halfWay2 = normalize(lightDir2 + viewDir); // Calculate the halfway vector for the second light

    //calculate specular coefficient
    float sC = pow(max(dot(normal, halfWay), 0.0), MatShine);
    float sC2 = pow(max(dot(normal, halfWay2), 0.0), MatShine); // Calculate the specular coefficient for the second light

    // Calculate the diffuse coefficient, needs to be greater than 0
    float dC = max(dot(normal, light), 0.0);
    float dC2 = max(dot(normal, light2), 0.0); // Calculate the diffuse coefficient for the second light

    // Calculate the final color
    color = vec4(MatAmb + lightIntensity * (dC * MatDif + sC * MatSpec) + lightIntensity2 * (dC2 * MatDif + sC2 * MatSpec), 1.0);
}