/*
	Program to render a cube in OpenGL, either using a filled-in model or wireframe model.
*/


#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <string.h>
#include <vector>

//================
//GLOBAL VARIABLES
//================
GLuint programID;
GLint attribute_coord3d, attribute_v_color, uniform_mvp;
GLuint vbo_verticies, vbo_color;
GLuint ibo_elements;
GLint compileOK, linkOK = GL_FALSE;

int screenWidth = 600;
int screenHeight = 600;

//load a shader from a file into a string so that openGL can use it.
void loadShader(const std::string &shaderFile, GLuint id) {
	std::string line;
	std::string fileContents = "";

	std::ifstream file;
	file.open(shaderFile);
	if(file.fail()) {
		perror(shaderFile.c_str());
		exit(1);
	}
	//load the shader file
	while(std::getline(file, line)) {
		fileContents += line + '\n';
	}
	file.close();

	//compile shader
	const char* contentsPtr = fileContents.c_str();
	glShaderSource(id, 1, &contentsPtr, NULL);
	glCompileShader(id);
	glGetShaderiv(id, GL_COMPILE_STATUS, &compileOK);

	//if compilation failed, print a detailed error message.
	if(!compileOK) {
		std::cerr << "ERROR: " << shaderFile << " FAILED TO COMPILE\n";
		GLint maxLength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<char> errorLog(maxLength);
		glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);
		glDeleteShader(id);
		std::printf("%s\n", &errorLog[0]);
		exit(1);
	}
}

bool initResources() {

	//VERTICIES
	GLfloat verticies[] = {
		//front of cube
		-1.0, -1.0, 1.0,
	     1.0, -1.0, 1.0,
		 1.0,  1.0, 1.0,
		-1.0,  1.0, 1.0,
		//back of cube
		-1.0,-1.0, -1.0,
	     1.0,-1.0, -1.0,
		 1.0, 1.0,  -1.0,
		-1.0, 1.0, -1.0
	};

	glGenBuffers(1, &vbo_verticies);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_verticies);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);


	//COLORS
	GLfloat color[] = {

		//front colors
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 1.0, 1.0,

		//back colors
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 1.0, 1.0


/*
		//front colors for wireframe mode
		1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,

		//back colors for wireframe mode
		1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
		1.0, 1.0, 1.0
*/
	};

	//INDEX BUFFER
	GLushort cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
	glGenBuffers(1, &ibo_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);



	//=============
	// LOAD SHADER
	//=============

	GLuint vertexID = glCreateShader(GL_VERTEX_SHADER);
	loadShader("CubeVertexShader.glsl", vertexID);


	GLuint fragID = glCreateShader(GL_FRAGMENT_SHADER);
	loadShader("CubeFragShader.glsl", fragID);


	//=================
	// PROGRAM LINKING
	//=================

	programID = glCreateProgram();
	glAttachShader(programID, vertexID);
	glAttachShader(programID, fragID);
	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &linkOK);
	if(!linkOK) {
		std::cerr << " ERROR: Could not link program!\n";
		return false;
	}

	//==========
	//ATTRIBUTES
	//==========

	const char* attributeName = "coord3d";
	attribute_coord3d = glGetAttribLocation(programID, attributeName);
	if(attribute_coord3d == -1) {
		std::cerr << "Could not bind attribute: " << attributeName << std::endl;
		return false;
	}

	attributeName = "v_color";
	attribute_v_color = glGetAttribLocation(programID, attributeName);
	if(attribute_v_color == -1) {
		std::cerr << "Could not bind attribute: " << attributeName << std::endl;
		return false;
	}

	//========
	//UNIFORMS
	//========

	const char* uniformName = "mvp";
	uniform_mvp = glGetUniformLocation(programID, uniformName);
	if(uniform_mvp == -1) {
		std::cerr << "Could not bind uniform: " << uniformName << std::endl;
		return false;
	}

	return true;
}

void render(SDL_Window* window) {
	//wireframe mode - comment the line below to see it filled in
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//clear the background to black
	glClearColor(0.0, 0.0, 0.0, 1.0);

    glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_verticies);
	glEnableVertexAttribArray(attribute_coord3d);

	glVertexAttribPointer(
	attribute_coord3d, //name of attribute
	3, //number of attributes per vertex (x, y, and z in this case)
	GL_FLOAT, //type of the attribute
	GL_FALSE, //take our values as is
	0, //no extra data between positions
	0 //offset of first position
	);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glEnableVertexAttribArray(attribute_v_color);
	glVertexAttribPointer(attribute_v_color, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//draw the cube
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
	int bufferSize;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
	glDrawElements(GL_TRIANGLES, bufferSize/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);


	//push each element to the vertex shader (6 because it has to draw 2 triangles to make a square)
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_v_color);

	//display the result
	SDL_GL_SwapWindow(window);
}

void logic() {

	//rotation animation
	float angle = SDL_GetTicks() / 1000.0 * 35; //35 degrees per second
	glm::vec3 axisY(0, 1, 0);
	glm::vec3 axisZ(0, 0, 1);
	glm::vec3 axisX(1, 0, 0);

	//default value is axisY, animation now tumbles the cube instead of just rotating it!
	//it tumbles it by multiplying the y axis with the x axis and with the z axis, allowing it to turn in 3D!
	glm::mat4 animation = glm::rotate(glm::mat4(1.0), glm::radians(angle), axisY) *
	glm::rotate(glm::mat4(1.0), glm::radians(angle), axisX) *
	glm::rotate(glm::mat4(1.0), glm::radians(angle), axisZ);

	//elements of the mvp matrix
	glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0, 0.0, -4.0));
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 1.0, 0.0));
	//project at a 45 degree FOV
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f * screenWidth / screenHeight, 0.1f, 10.0f);

	//model view projection matrix, with rotation
	glm::mat4 mvp = projection * view * model * animation;

	glUseProgram(programID);
	//tell OpenGL where the uniform matrix is in the shader. (mvp, in this case)
	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
}

//loop and process events
void mainLoop(SDL_Window* window) {
	while(true) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				return;
			} else if(event.type == SDL_KEYDOWN) {
			//if the end key is pressed, end the program
				switch(event.key.keysym.sym) {
					case SDLK_END:
					exit(0);
					break;
				}
			}
		}
		logic();
		render(window);
	}
}

//clean up used memory
void freeResources() {
	glDeleteProgram(programID);
	glDeleteBuffers(1, &vbo_verticies);
}

int main() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("First Cube", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(window);

	GLenum glewStatus = glewInit();
	if(glewStatus != GLEW_OK) {
		std::cerr << glewGetErrorString(glewStatus) << std::endl;
	}

	if(window == nullptr) {
		std::cerr << SDL_GetError() << std::endl;
		exit(1);
	}

	if(!initResources()) {
		std::cerr << "initResources failed!\n";
		exit(1);
	}


	mainLoop(window);
	freeResources();
	return 0;
	}
