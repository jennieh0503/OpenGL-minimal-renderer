#include<iostream>
#include<GLAD/glad.h>
#include<GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include"shaderClass.h"
#define ACCELERATION 1.0f

typedef struct mesh{
	unsigned int VBO;
	unsigned int VAO;
	unsigned int shaderProg;
} Mesh;

GLFWwindow* initialize();
void drawShape(GLFWwindow* window, Mesh m);
Mesh initializeShape();
void update(float* offset, float* velocity, float deltaTime);




int main() {

	//initialize window including glfw
	GLFWwindow* window = initialize();
	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//set window as context
	glfwMakeContextCurrent(window);

	//start glad
	gladLoadGL();

	//set working or display area
	//the function requires coordinates for bottom left corner and top left corner of the interested area
	//In this experiment I used the entire window
	glViewport(0, 0, 800, 600);

	//initializing shape with buffers and shaders, sending data to gpu to avoid repeated work conuming time
	Mesh m = initializeShape();
	//initializing offset and velocity for the shape movement/update logic
	float offset[] = { 0.0f, 0.0f, 0.0f };
	float velocity[] = { 0.0f, 1.0f, 0.0f };
	float time = glfwGetTime();
	
	//let window run until closure, added logic for a "bouncy" shape
	while (!glfwWindowShouldClose(window)) {

		//update the position during runtime
		float newTime = glfwGetTime();
		float deltaTime = newTime - time;
		time = newTime;
		update(offset, velocity, deltaTime);

		//create transformation matrix
		float transform[4][4] = {
			{1.0f, 0.0f, 0.0f, offset[0]},
			{0.0f, 1.0f, 0.0f, offset[1]},
			{0.0f, 0.0f, 1.0f, offset[2]},
			{0.0f, 0.0f, 0.0f, 1.0f}
		};

		//pass transformation matrix to vertex shader
		unsigned int transformationLoc = glGetUniformLocation(m.shaderProg, "transformation");
		glUseProgram(m.shaderProg);
		glUniformMatrix4fv(transformationLoc, 1, GL_TRUE, &transform[0][0]);

		drawShape(window, m);

		glfwPollEvents();
	}


	//clean used resources
	glDeleteVertexArrays(1, &m.VAO);
	glDeleteBuffers(1, &m.VBO);
	glDeleteProgram(m.shaderProg);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


GLFWwindow* initialize() {
	//initialize glfw
	glfwInit();

	//initialize window, specify version (3.3 or higher in this case)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create window and check errors
	//glfw creation functions requires width, length and name as parameters
	//there are two extra parameters that are monitor and window, that I am going to ignore for the scope of this project
	GLFWwindow* window = glfwCreateWindow(800, 600, "minimalRender", NULL, NULL);

	return window;
}

Mesh initializeShape() {
	Mesh m;

	//create vertices array, the array contains coordinates in normalized form
	//z axis kept to a constant to represent a flat 2d object
	float vertices[] = {
		-0.5f, -1.0f, 0.0f,
		0.5f, -1.0f, 0.0f,
		-0.5f, 0.0f, 0.0f,
		0.5f, -1.0f, 0.0f,
		0.5f, 0.0f, 0.0f,
		-0.5f, 0.0f, 0.0f
	};


	//creating shaders using the shader class
	Shader sProg("vert.txt", "frag.txt");

	m.shaderProg = sProg.ID;

	//vertices need to be passed to the gpu, buffers are used to send large amounts without having to consume more time
	//creating a vertix buffer (not using an array because of only one object present to send)
	//a vertex array object is also needed to switch between objects faster
	glGenVertexArrays(1, &m.VAO);
	glGenBuffers(1, &m.VBO);
	glBindVertexArray(m.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	return m;
}



//added offset to make the shape move, offset not checked, it could end out the visible window
void drawShape(GLFWwindow* window, Mesh m) {
	//configuration of VAO to interpret the data sent to the buffer.
	//the vertices were saved as sets of coordinates, all in one single array, therefore each 3 belond to a vertex
	//(starting from element 0 of the array vertices)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//draw shape
	//changed int0 a rectangle, obtained as two triangles so the number of vertices became 6
	glClearColor(0.924, 0.929, 0.920, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(m.shaderProg);
	glBindVertexArray(m.VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glfwSwapBuffers(window);
}

//update logic
void update(float* offset, float* velocity, float deltaTime) {
	//update the offset to achieve movement
	offset[1] += velocity[1]*deltaTime;

	//boundaries check, invert velocity when hitting a boundary
	if (offset[1] > 1.0f || offset[1] < 0.0f) {
		velocity[1] = -velocity[1];
	}

	//velocity clamping
	if (velocity[1] > 1.0f) {
		velocity[1] = 1.0f;
	}
	else if (velocity[1] < -1.0f) {
		velocity[1] = -1.0f;
	}

	//velocity logic, slows down over time going up until it becomes slow enough to start going down, accelerates going down
	//could be further developed with actual gravity and friction
	velocity[1] -= ACCELERATION * deltaTime;

}