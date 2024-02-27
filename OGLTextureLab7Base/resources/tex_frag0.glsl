#version 330 core
uniform sampler2D Texture0;
uniform int flip;


in vec3 fragNor;
in vec3 lightDir;
in vec2 vTexCoord;
out vec4 Outcolor;

void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);

  vec3 norm = flip == 1? -normalize(fragNor) : normalize(fragNor);
  vec3 lightDirection = normalize(lightDir);

  float diff = max(dot(norm, lightDirection), 0.0);

  Outcolor = diff * texColor0;

  if (Outcolor.b > Outcolor.r || Outcolor.b > Outcolor.g) {
    discard;
  }
}