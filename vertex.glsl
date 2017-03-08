// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexColour;
layout(location = 2) in vec2 VertexTextureCoords;

// output to be interpolated between vertices and passed to the fragment stage
out vec3 Colour;
out vec2 TextureCoords;
out vec3 N;
out vec3 L;
out vec3 V;

// uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 location;
uniform vec3 cam;

void main()
{
    // assign vertex position without modification
    gl_Position = proj*view*model*vec4(VertexPosition, 1.0);

    // assign output colour to be interpolated
    Colour = VertexColour;
    TextureCoords = VertexTextureCoords;

    // update lighting vectors
    N = vec3(normalize(VertexPosition.x - location.x), normalize(VertexPosition.y - location.y), normalize(VertexPosition.z - location.z));
    L = normalize(vec3(VertexPosition.x,VertexPosition.y,VertexPosition.z));
    V = normalize(vec3(cam.x,cam.y,cam.z));
}
