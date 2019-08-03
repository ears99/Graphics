#include <iostream>
#include <cstdlib>
#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL2/SDL.h>

GLuint programID;
GLint attribute_coord2d;

bool initResources(void) {
	GLint compileOK = GL_FALSE;
	GLint linkOK = GL_FALSE;

	//vertex shader
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexSource =
	"#version 120\n"
	"attribute vec2 coord2d;\n"
	"void main() {"
		"gl_Position = vec4(coord2d, 0.0, 1.0);"
	"}";
	glShaderSource(vertexShaderID, 1, &vertexSource, NULL);
	glCompileShader(vertexShaderID);
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &compileOK);
	if(!compileOK) {
		std::cerr << "Error in vertex shader!\n";
		return false;
	}

	//fragment shader
	GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragSource =
	"#version 120\n"
	"void main() {"
		"gl_FragColor[0] = gl_FragCoord.x/600;" //red
		"gl_FragColor[1] = gl_FragCoord.y/600;" //green
		"gl_FragColor[2] = 0.5;" //blue
	"}";
	glShaderSource(fragShaderID, 1, &fragSource, NULL);
	glCompileShader(fragShaderID);
	glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &compileOK);
	if(!compileOK) {
		std::cerr << "Error in fragment shader!";
		return false;
	}
	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragShaderID);
	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &linkOK);
	if(!linkOK) {
		std::cerr << "Program failed to link!";
		return false;
	}
	const char* attributeName = "coord2d";
	attribute_coord2d = glGetAttribLocation(programID, attributeName);
	if(attribute_coord2d == -1) {
		std::cerr << "Could not bind attribute " << attributeName << std::endl;
		return false;
	}
	return true;
}

void render(SDL_Window* window) {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programID);
	glEnableVertexAttribArray(attribute_coord2d);
	GLfloat verticies[] = {
		0.0,  0.5,
	   -0.5, -0.5,
		0.5, -0.5
	};

	glVertexAttribPointer(
		attribute_coord2d, //name of attribute
		2, //number of elements per vertex
		GL_FLOAT, 		  //type of each element
		GL_FALSE, 		 //take the values as is
		0, 		  		//no extra data between the positions
		verticies 	   //pointer to the array
	);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(attribute_coord2d);
	SDL_GL_SwapWindow(window);
}


void freeResources() {
	glDeleteProgram(programID);
}


void mainLoop(SDL_Window* window) {
	while(true) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				return;
			}
			render(window);
		}
	}
}

int main(int argc, char** argv) {
//initialize SDL
		SDL_Init(SDL_INIT_VIDEO);
		SDL_Window* window = SDL_CreateWindow("First triangle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 600, SDL_WINDOW_OPENGL);
		SDL_GL_CreateContext(window);

//initialize glew
		GLenum glewStatus = glewInit();
		if(glewStatus != GLEW_OK) {
			std::cerr << "GLEW Error: " << glewGetErrorString(glewStatus);
			return 1;
		}

		if(!initResources()) {
			std::cerr << "Error: initResources failed to initialize!";
			return 1;
		}

		mainLoop(window);
		freeResources();

	return 0;
}
