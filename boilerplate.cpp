// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

float speed = 1.0;
int earthType = 1;

// Specify that we want the OpenGL core profile before including GLFW headers
#ifndef LAB_LINUX
	#include <glad/glad.h>
#else
	#define GLFW_INCLUDE_GLCOREARB
	#define GL_GLEXT_PROTOTYPES
#endif
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define PI 3.14159265359

using namespace std;
using namespace glm;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
	// OpenGL names for vertex and fragment shaders, shader program
	GLuint  vertex;
	GLuint  fragment;
	GLuint  program;

	// initialize shader and program names to zero (OpenGL reserved value)
	MyShader() : vertex(0), fragment(0), program(0)
	{}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
	// load shader source from files
	string vertexSource = LoadSource("vertex.glsl");
	string fragmentSource = LoadSource("fragment.glsl");
	if (vertexSource.empty() || fragmentSource.empty()) return false;

	// compile shader source into shader objects
	shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
	shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	// link shader program
	shader->program = LinkProgram(shader->vertex, shader->fragment);

	// check for OpenGL errors and return false if error occurred
	return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
	// unbind any shader programs and destroy shader objects
	glUseProgram(0);
	glDeleteProgram(shader->program);
	glDeleteShader(shader->vertex);
	glDeleteShader(shader->fragment);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing textures

struct MyTexture
{
	GLuint textureID;
	GLuint target;
	int width;
	int height;

	// initialize object names to zero (OpenGL reserved value)
	MyTexture() : textureID(0), target(0), width(0), height(0)
	{}
};

bool InitializeTexture(MyTexture* texture, const char* filename, GLuint target = GL_TEXTURE_2D)
{
	int numComponents;
	unsigned char *data = stbi_load(filename, &texture->width, &texture->height, &numComponents, 0);
	if (data != nullptr)
	{
		texture->target = target;
		glGenTextures(1, &texture->textureID);
		glBindTexture(texture->target, texture->textureID);
		GLuint format = GL_RGB;
		switch(numComponents)
		{
			case 4:
				format = GL_RGBA;
				break;
			case 3:
				format = GL_RGB;
				break;
			case 2:
				format = GL_RG;
				break;
			case 1:
				format = GL_RED;
				break;
			default:
				cout << "Invalid Texture Format" << endl;
				break;
			};
		glTexImage2D(texture->target, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, data);

		// Note: Only wrapping modes supported for GL_TEXTURE_RECTANGLE when defining
		// GL_TEXTURE_WRAP are GL_CLAMP_TO_EDGE or GL_CLAMP_TO_BORDER
		glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Clean up
		glBindTexture(texture->target, 0);
		stbi_image_free(data);
		return !CheckGLErrors();
	}
	return true; //error
}

// deallocate texture-related objects
void DestroyTexture(MyTexture *texture)
{
	glBindTexture(texture->target, 0);
	glDeleteTextures(1, &texture->textureID);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
	// OpenGL names for array buffer objects, vertex array object
	GLuint  vertexBuffer;
	GLuint  textureBuffer;
	GLuint  colourBuffer;
	GLuint  elementBuffer;
	GLuint  vertexArray;
	GLsizei elementCount;

	// initialize object names to zero (OpenGL reserved value)
	MyGeometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
	{}
};

// create buffers and fill with geometry data, returning true if successful
bool InitializeGeometry(MyGeometry *geometry)
{
	float divisions = 360;//48
	float angle = 360/(divisions-1);
	float radius = 1.0;
	int vertexCount = 0;
	float longitude = 0.f;
	float latitude = 0.f;
	vec3 point(0.f,radius,0.f);

	GLfloat vertices[(int)(divisions*divisions)][3];
	GLfloat colours[(int)(divisions*divisions)][3];
	unsigned indices[(int)(6*divisions*divisions)];
	float textureCoords[(int)(divisions*divisions)][2];

	for(float u = 0; u < divisions; u++) {
		for(float v = 0; v < divisions; v++) {
			float theta = u*angle;
			float phi = v*angle*0.5;
			point = vec3(radius*cos(theta*PI/180)*sin(phi*PI/180), radius*sin(theta*PI/180)*sin(phi*PI/180), radius*cos(phi*PI/180));
			vertices[vertexCount][0] = point.x; vertices[vertexCount][1] = point.z; vertices[vertexCount][2] = point.y;

			longitude = (theta)/(360);
			latitude = (phi)/(180);

			textureCoords[vertexCount][0] = longitude;
			textureCoords[vertexCount][1] = latitude;

			colours[vertexCount][0] = 0.0; colours[vertexCount][1] = 1-1/(float)vertexCount; colours[vertexCount][2] = 1-1/(float)vertexCount-.5;
			vertexCount++;
		}
	}


	int indexCount = 0;
	for(int i=0; i<divisions-1; i++)
	{
		for(int j=0; j<divisions-1; j++)
		{
			unsigned int p00 = i*divisions+j;
			unsigned int p01 = i*divisions+j+1;
			unsigned int p10 = (i+1)*divisions + j;
			unsigned int p11 = (i+1)*divisions + j + 1;

			indices[indexCount]= p00; indexCount++;
			indices[indexCount]= p10; indexCount++;
			indices[indexCount]= p01; indexCount++;

			indices[indexCount]= p01; indexCount++;
			indices[indexCount]= p10; indexCount++;
			indices[indexCount]= p11; indexCount++;
		}
	}

	geometry->elementCount = indexCount;
	//geometry->elementCount = 36;

	// these vertex attribute indices correspond to those specified for the
	// input variables in the vertex shader
	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;
	const GLuint TEXTURE_INDEX = 2;

	// create an array buffer object for storing our vertices
	glGenBuffers(1, &geometry->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// create another one for storing our colours
	glGenBuffers(1, &geometry->colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);

	glGenBuffers(1, &geometry->textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

	// create a vertex array object encapsulating all our vertex attributes
	glGenVertexArrays(1, &geometry->vertexArray);
	glBindVertexArray(geometry->vertexArray);

	glGenBuffers(1, &geometry->elementBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->elementBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// associate the position array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
	glVertexAttribPointer(TEXTURE_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(TEXTURE_INDEX);

	// assocaite the colour array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(COLOUR_INDEX);

	// unbind our buffers, resetting to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// check for OpenGL errors and return false if error occurred
	return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
	// unbind and destroy our vertex array object and associated buffers
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &geometry->vertexArray);
	glDeleteBuffers(1, &geometry->vertexBuffer);
	glDeleteBuffers(1, &geometry->colourBuffer);
	glDeleteBuffers(1, &geometry->elementBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(MyGeometry *geometry, MyShader *shader, MyTexture* t)
{
	// bind our shader program and the vertex array object containing our
	// scene geometry, then tell OpenGL to draw our geometry
	glBindTexture(t->target, t->textureID);
	glUseProgram(shader->program);
	glBindVertexArray(geometry->vertexArray);
	glDrawElements(GL_TRIANGLES,geometry->elementCount,GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_TRIANGLES, 0, geometry->elementCount);

	// reset state to default (no shader or geometry bound)
	glBindTexture(t->target, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	// check for an report any OpenGL errors
	CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GL_TRUE);
	} else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		speed = speed*2;
	} else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		if (speed > 0.0)
			speed = speed/2;
	} else if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		if (earthType > 2){
			earthType = 1;
		} else {
			earthType++;
		}
	}
}

void updateCamera(vec3& cameraLoc, GLFWwindow* window)
{
		float theta = 1.0*PI/180;
		float scale = 0.1;

		//Side to side movement
		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraLoc = vec3((cameraLoc.x * cos(theta)) - (cameraLoc.z * sin(theta)),cameraLoc.y,(cameraLoc.z * cos(theta)) + (cameraLoc.x * sin(theta)));
		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraLoc = vec3((cameraLoc.x * cos(-theta)) - (cameraLoc.z * sin(-theta)),cameraLoc.y,(cameraLoc.z * cos(-theta)) + (cameraLoc.x * sin(-theta)));

		//Front and back movement
		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraLoc = vec3(cameraLoc.x,cameraLoc.y,cameraLoc.z+scale);
		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraLoc = vec3(cameraLoc.x,cameraLoc.y,cameraLoc.z-scale);

		//Up and Down movement
		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			cameraLoc = vec3(cameraLoc.x,cameraLoc.y-theta,cameraLoc.z);
		if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			cameraLoc = vec3(cameraLoc.x,cameraLoc.y+theta,cameraLoc.z);

}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);

	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	int width = 1920, height = 1080;
	window = glfwCreateWindow(width, height, "Solar System", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}

	// set keyboard callback function and make our context current (active)
	glfwSetKeyCallback(window, KeyCallback);
	glfwMakeContextCurrent(window);

	//Intialize GLAD
	#ifndef LAB_LINUX
	if (!gladLoadGL())
	{
		cout << "GLAD init failed" << endl;
		return -1;
	}
	#endif

	// query and print out information about our OpenGL environment
	QueryGLVersion();

	// call function to load and compile shader programs
	MyShader shader;
	if (!InitializeShaders(&shader)) {
		cout << "Program could not initialize shaders, TERMINATING" << endl;
		return -1;
	}

	// call function to create and fill buffers with geometry data
	MyGeometry geometry;
	if (!InitializeGeometry(&geometry))
		cout << "Program failed to intialize geometry!" << endl;

	//toggle wireframe
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	float sunAngle = 0.f,   		 sunScale = 1.f,                  sunRotation = (2*PI)/25.38f,        sunTilt = 0.1265364f;
	float earthAngle = 0.f, 		 earthScale = 0.49069689823f, 	  earthRotation = (2*PI)/0.99726968f,
				earthOrbitAngle = 0.f, earthOrbit = (2*PI)/365.006351f, earthDistance = 8.17492546808f,     earthTilt = 0.40910518f;
	float moonAngle = 0.f,       moonScale = 0.38420392891f,      moonRotation = (2*PI)/27.321582f,
				moonOrbitAngle = 0.f,  moonOrbit = (2*PI)/27.32158f,    moonDistance = 5.5847822492f, 			moonTilt = 0.116588f,
				moonInclination = 0.f;

	vec3 sunLocation(0,0,0);
	vec3 earthLocation(earthDistance,0,0);
	vec3 moonLocation(moonDistance,0,0);
	vec3 earthAxis(0,1,0);
	vec3 sunAxis(0,1,0);
	vec3 moonAxis(0,1,0);

	vec3 cameraLoc(0,0,20);
	vec3 cameraUp(0,1,0);

  float aspectRatio = (float)width/ (float)height;
	float zNear = .1f, zFar = 20000.f;
	float fov = 1.0472f;

	mat4 I(1);
	glUseProgram(shader.program);
	GLint modelUniform = glGetUniformLocation(shader.program, "model");
	GLint viewUniform = glGetUniformLocation(shader.program, "view");
	GLint projUniform = glGetUniformLocation(shader.program, "proj");

	//==============
	// clear screen to a dark grey colour
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glfwSetCursorPos(window, width/2, height/2); //centre the mouse
	//double phi = 3.14159265, theta = 3.14159265 / 2.0;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); //disable with GLFW_CURSOR_NORMAL
	//=============

	MyTexture sunTexture, earthTexture, earthTextureNoclouds, earthTextureNight, moonTexture, starTexture;
	InitializeTexture(&sunTexture, "Sun_Map.jpg");
	InitializeTexture(&earthTexture, "Earth_Map.jpg");
	InitializeTexture(&earthTextureNoclouds, "Earth_Map_Noclouds.jpg");
	InitializeTexture(&earthTextureNight, "Earth_Map_Night.jpg");
	InitializeTexture(&moonTexture, "Moon_Map.jpg");
	InitializeTexture(&starTexture, "Star_Map.jpg");

	glfwSetWindowPos(window, 3000, 400);

	glfwSetTime(0.0);
	//double time = 0.0;
	double timechange = 0.0;

	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //=======

		timechange = glfwGetTime();
		glfwSetTime(0.0);

		//sun
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    sunAngle = sunAngle + timechange*sunRotation*speed;
		glUseProgram(shader.program);
		mat4 model = rotate(I, sunTilt, vec3(0,0,1)) * translate(I, sunLocation) * rotate(I, sunAngle, sunAxis) * scale(I, vec3(sunScale, sunScale, sunScale));
		//Update camera pos
		updateCamera(cameraLoc, window);

		mat4 view = lookAt(cameraLoc, vec3(0,0,0), cameraUp);
		mat4 proj = perspective(fov, aspectRatio, zNear, zFar);
		glUniformMatrix4fv(modelUniform, 1, false, value_ptr(model));
		glUniformMatrix4fv(viewUniform, 1, false, value_ptr(view));
		glUniformMatrix4fv(projUniform, 1, false, value_ptr(proj));
		// call function to draw our scene
		RenderScene(&geometry, &shader , &sunTexture);

		//Earth
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  	earthAngle = earthAngle + timechange*earthRotation*speed;
		earthOrbitAngle = timechange*(-earthOrbit)*speed;
		earthLocation = vec3( ((earthLocation.x * cos(earthOrbitAngle)) - (earthLocation.z * sin(earthOrbitAngle))), earthLocation.y, ((earthLocation.z * cos(earthOrbitAngle)) + (earthLocation.x * sin(earthOrbitAngle))) );
		glUseProgram(shader.program);
		model = translate(I, earthLocation) * rotate(I, earthTilt, vec3(0,0,1)) * rotate(I, earthAngle, earthAxis) * scale(I, vec3( earthScale,  earthScale, earthScale));
		glUniformMatrix4fv(modelUniform, 1, false, value_ptr(model));
		switch (earthType) {
			case 1:
			RenderScene(&geometry, &shader , &earthTexture);
			break;
			case 2:
			RenderScene(&geometry, &shader , &earthTextureNoclouds);
			break;
			case 3:
			RenderScene(&geometry, &shader , &earthTextureNight);
			break;
		}

		//Moon
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		moonAngle = moonAngle + timechange*moonRotation*speed;
		glUseProgram(shader.program);

		moonLocation = vec3(moonDistance,0,0);
		model = translate(I, moonLocation);
		moonOrbitAngle = moonOrbitAngle + timechange*(-moonOrbit)*speed;
		moonLocation = vec3( ((moonLocation.x * cos(moonOrbitAngle)) - (moonLocation.z * sin(moonOrbitAngle))), moonLocation.y + moonInclination, ((moonLocation.z * cos(moonOrbitAngle)) + (moonLocation.x * sin(moonOrbitAngle))) );
		model = translate(I, moonLocation);
		moonLocation = vec3(earthLocation.x + moonLocation.x, earthLocation.y, earthLocation.z + moonLocation.z);
		moonInclination = (moonLocation.x - earthLocation.x) * moonDistance * 0.08979719;
		moonLocation = vec3(moonLocation.x, earthLocation.y + moonInclination, moonLocation.z);
		model = translate(I, moonLocation) * rotate(I, moonTilt, vec3(0,0,1)) * rotate(I, moonAngle, moonAxis) * scale(I, vec3(moonScale, moonScale, moonScale));
		glUniformMatrix4fv(modelUniform, 1, false, value_ptr(model));
		RenderScene(&geometry, &shader , &moonTexture);

		//stars
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUseProgram(shader.program);
		model = scale(I, vec3(10000, 10000, 10000));
		glUniformMatrix4fv(modelUniform, 1, false, value_ptr(model));
		RenderScene(&geometry, &shader , &starTexture);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// clean up allocated resources before exit
	DestroyGeometry(&geometry);
	DestroyShaders(&shader);
	glfwDestroyWindow(window);
	glfwTerminate();

	cout << "Goodbye!" << endl;
	return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
	bool error = false;
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		cout << "OpenGL ERROR:  ";
		switch (flag) {
		case GL_INVALID_ENUM:
			cout << "GL_INVALID_ENUM" << endl; break;
		case GL_INVALID_VALUE:
			cout << "GL_INVALID_VALUE" << endl; break;
		case GL_INVALID_OPERATION:
			cout << "GL_INVALID_OPERATION" << endl; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
		case GL_OUT_OF_MEMORY:
			cout << "GL_OUT_OF_MEMORY" << endl; break;
		default:
			cout << "[unknown error code]" << endl;
		}
		error = true;
	}
	return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
	string source;

	ifstream input(filename.c_str());
	if (input) {
		copy(istreambuf_iterator<char>(input),
			istreambuf_iterator<char>(),
			back_inserter(source));
		input.close();
	}
	else {
		cout << "ERROR: Could not load shader source from file "
			<< filename << endl;
	}

	return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
	// allocate shader object name
	GLuint shaderObject = glCreateShader(shaderType);

	// try compiling the source as a shader of the given type
	const GLchar *source_ptr = source.c_str();
	glShaderSource(shaderObject, 1, &source_ptr, 0);
	glCompileShader(shaderObject);

	// retrieve compile status
	GLint status;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
		cout << "ERROR compiling shader:" << endl << endl;
		cout << source << endl;
		cout << info << endl;
	}

	return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	// allocate program object name
	GLuint programObject = glCreateProgram();

	// attach provided shader objects to this program
	if (vertexShader)   glAttachShader(programObject, vertexShader);
	if (fragmentShader) glAttachShader(programObject, fragmentShader);

	// try linking the program with given attachments
	glLinkProgram(programObject);

	// retrieve link status
	GLint status;
	glGetProgramiv(programObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
		cout << "ERROR linking shader program:" << endl;
		cout << info << endl;
	}

	return programObject;
}
