#version 330 core
uniform sampler2D Texture0;


in vec3 fragNor;
in vec3 lightDir;
in vec2 vTexCoord;
flat in int isGroundFrag;
out vec4 Outcolor;

void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);

  vec3 norm = -normalize(fragNor);
  vec3 lightDirection = normalize(lightDir);

  float diff = max(dot(norm, lightDirection), 0.0);

  Outcolor = diff * texColor0 * (isGroundFrag == 1 ? 0.7 : 1);
}