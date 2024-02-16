#version 330 core
uniform sampler2D Texture0;

in vec2 vTexCoord;
out vec4 Outcolor;

void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);

  	//to set the out color as the texture color 
  	Outcolor = texColor0;
  
  	//to set the outcolor as the texture coordinate (for debugging)
	//Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
}

