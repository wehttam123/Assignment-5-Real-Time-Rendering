// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// interpolated colour received from vertex stage
in vec3 Colour;
in vec2 TextureCoords;
in vec3 N;
in vec3 L;
in vec3 V;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

uniform sampler2D tex;
uniform vec2 lighting;

void main(void)
{
  float amb = 0.75f;
  float dif = clamp(dot(N,L),0.0f,1.0f);
  float spec = 0.5f;
  float P = 1;
  vec3 H = (V + L)/length(V + L);

  if (lighting.x == 0){
    FragmentColour = (texture(tex, TextureCoords) * amb) + (texture(tex, TextureCoords) * 0.01 * max(0,dot(N,L))) + (spec * 0.01 * pow(max(0,dot(N,H)),P));
  } else {
    FragmentColour = texture(tex, TextureCoords);
  }
}
