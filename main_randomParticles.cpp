#include <iostream>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaderClass.h"

#define NUM_POINTS_IN_CIRCLE 32
#define NUM_PARTICLES 50
#define MAX_ATTEMPTS 100
#define RESTITUTION 0.9f  //adding damping and energy loss in collisions for realism

typedef struct mesh{
	unsigned int VBO;
	unsigned int VAO;
	unsigned int shaderProg;
} Mesh;

typedef struct particle {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 force;

	float radius;
	float mass;
};

GLFWwindow* initialize();
void draw(GLFWwindow* window, Mesh m, std::vector<particle> particles);
Mesh initializeShape();
void updateParticle(particle& p, glm::vec3 gravity, float deltaTime);
bool checkCollision(particle& p1, particle& p2);
void resolveCollision(particle& p1, particle& p2);
void updatePositions(particle& p1, particle& p2);
float randomFloat(float min, float max);
bool overlaps(particle& p, std::vector<particle>& particles);
particle generateParticle();



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

	//initializing particles
	std::vector<particle> particles;
	//populating particles with random generation
	for (int i = 0; i < NUM_PARTICLES; i++) {
		particle p;
		
		for (int j = 0; j < MAX_ATTEMPTS; j++) {  //limit attempts to avoid infinite loops
			p = generateParticle();
			if (!overlaps(p, particles)) {
				particles.push_back(p);
				break;
			}
		}
	}


	//defining gravity and initializing time
	const glm::vec3 gravity(0.0f, -9.81f, 0.0f);
	float time = glfwGetTime();
	
	//let window run until closure, added logic for a "bouncy" shape
	while (!glfwWindowShouldClose(window)) {

		//update delta time (no longer frame dependent)
		float newTime = glfwGetTime();
		float deltaTime = newTime - time;
		time = newTime;

		//update each particle by applying gravity (also checking for boundaries)
		for (particle& p : particles) {
			updateParticle(p, gravity, deltaTime);
		}

		//update with collisions
		for (particle& p1 : particles) {
			for (particle& p2 : particles) {
				if (checkCollision(p1, p2)) {
					resolveCollision(p1, p2);
				}
			}
		}

		draw(window, m, particles);

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


//initializing glfw, window
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


//shape initialization function, creates the shape by sending data to the gpu and creating shader
//updated to create circles (particles simplification for collision simulation)
//updated by using glm for vector operations
Mesh initializeShape() {
	Mesh m;

	float angle = 360.0f / NUM_POINTS_IN_CIRCLE;

	//create vertices vector
	//z axis kept to a constant to represent a flat 2d object
	std::vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f)); //center point
	for (int i = 0; i <= NUM_POINTS_IN_CIRCLE;  i++) {
		float currentAngle = angle * i;
		float x = 1.0f * cos(glm::radians(currentAngle));
		float y = 1.0f * sin(glm::radians(currentAngle));
		vertices.push_back(glm::vec3(x, y, 0.0f));  //2d, constant z
	}

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
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	//configuration of VAO to interpret the data sent to the buffer.
	//the vertices were saved as sets of coordinates, all in one single array, therefore each 3 belond to a vertex
	//(starting from element 0 of the array vertices)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	return m;
}



//drawing function
void draw(GLFWwindow* window, Mesh m, std::vector<particle> particles) {

	glClearColor(0.0891, 0.0873, 0.0900, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	unsigned int transformationLoc = glGetUniformLocation(m.shaderProg, "transformation"); //get location of transformation matrix in shader

	//update transformation matrix for each particle
	//because the standard shape has a radius of 1 scaling is done to get the particle's actual size
	//translation allows to showcase motion (without having to modify the actual vertex data)
	for (particle p : particles) {
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, p.position);
		transform = glm::scale(transform, glm::vec3(p.radius, p.radius, 1.0f));

		//pass transformation matrix to vertex shader
		glUseProgram(m.shaderProg);
		glUniformMatrix4fv(transformationLoc, 1, GL_FALSE, &transform[0][0]);

		//draw the particle
		glUseProgram(m.shaderProg);
		glBindVertexArray(m.VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_POINTS_IN_CIRCLE+2);
	}
	
	glfwSwapBuffers(window);
}

//update logic
//modified to simulate actual gravity and collision, not just uniformly accelerated motion
void updateParticle(particle& p, glm::vec3 gravity, float deltaTime) {
	p.force = gravity * p.mass;
	p.velocity += gravity * deltaTime;
	p.position += p.velocity * deltaTime;

	//boundaries
	if (p.position.x < -1.0f + p.radius) {
		p.position.x = -1.0f + p.radius;
		p.velocity.x *= -RESTITUTION;
	}
	else if (p.position.x > 1.0f - p.radius) {
		p.position.x = 1.0f - p.radius;
		p.velocity.x *= -RESTITUTION;
	}

	if (p.position.y < -1.0f + p.radius) {
		p.position.y = -1.0f + p.radius;
		p.velocity.y *= -RESTITUTION;
	}
	else if (p.position.y > 1.0f - p.radius) {
		p.position.y = 1.0f - p.radius;
		p.velocity.y *= -RESTITUTION;
	}
}

bool checkCollision(particle& p1, particle& p2) {
	float distance = glm::length(p1.position - p2.position);
	return distance < (p1.radius + p2.radius);
}

void resolveCollision(particle& p1, particle& p2) {
	//calculate normal vector, direction from p1 to p2, normalized to get only the direction without magnitude
	//represents the direction of the impulse
	glm::vec3 n = p2.position - p1.position;
	float distance = glm::length(n);
	if (distance == 0.0f) {
		return;
	}
	n = n / distance;

	//calculate relative velocity to see if the collision has already been solved and the particles are already going different ways
	glm::vec3 relativeVelocity = p2.velocity - p1.velocity;
	if (glm::dot(relativeVelocity, n) > 0) {  //checking the component on the normal, if it's positive they are moving apart
		return;
	}

	//calculate impulse using the formula for elastic collisions, modified by restitution to add damping
	float impulseMagnitude = -(1.0f + RESTITUTION) * glm::dot(relativeVelocity, n) / (1.0f / p1.mass + 1.0f / p2.mass);

	//apply impulse
	glm::vec3 impulse = impulseMagnitude * n;
	p1.velocity -= impulse / p1.mass;
	p2.velocity += impulse / p2.mass;

	updatePositions(p1, p2);

}

void updatePositions(particle& p1, particle& p2) {
	float percentage = 0.8f;  //percentage to move each particle, gradual corrections allow better stability and prevent jittering
	float slop = 0.01f;  //small value to prevent sinking due to numerical errors
	
	float distance = glm::length(p2.position - p1.position);
	if (distance == 0.0f) {
		return;
	}

	float penetration = (p1.radius + p2.radius) - distance;
	if (penetration > slop) {
		glm::vec3 correction = (penetration / (1.0f / p1.mass + 1.0f / p2.mass)) * ((p2.position - p1.position) / distance) * percentage;
		p1.position -= correction / p1.mass;
		p2.position += correction / p2.mass;
	}
}

float randomFloat(float min, float max) {
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

bool overlaps(particle& p, std::vector<particle>& particles) {
	for (particle& other : particles) {
		float distance = glm::length(p.position - other.position);
		if (distance < (p.radius + other.radius)) {
			return true;
		}
	}
	return false;
}

particle generateParticle() {
	particle p;

	p.radius = randomFloat(0.02f, 0.04f);
	p.mass = p.radius * p.radius;  //mass proportional to size for simplicity

	p.position = glm::vec3(randomFloat(-1.0f + p.radius, 1.0f - p.radius), randomFloat(-1.0f + p.radius, 1.0f - p.radius), 0.0f);
	p.velocity = glm::vec3(randomFloat(-0.5f, 0.5f), randomFloat(-0.5f, 0.5f), 0.0f);

	p.force = glm::vec3(0.0f);

	return p;
}