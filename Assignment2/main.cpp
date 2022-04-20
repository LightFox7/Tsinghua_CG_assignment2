#include <iostream>
#include <memory>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include <cmath>

// Other includes
#include "Shader.h"
#include "Sphere.h"

constexpr glm::vec3 GLM_UP(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 GLM_RIGHT(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 GLM_DOWN = -GLM_UP;
constexpr glm::vec3 GLM_LEFT = -GLM_RIGHT;

std::unique_ptr<glm::mat4> init_model = nullptr;
std::unique_ptr<glm::mat4> view = nullptr;
std::unique_ptr<glm::mat4> projection = nullptr;
std::unique_ptr<glm::mat4> model = nullptr;

glm::vec3 black = glm::vec3(0, 0, 0);
glm::vec3 white = glm::vec3(1, 1, 1);
glm::vec3 green = glm::vec3(0.1f, 0.95f, 0.1f);

const float PI = acos(-1);

std::vector<GLfloat> generateSphereTriangles()
{
	std::vector<GLfloat> triangles;

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> texCoords;

	float radius = 1.0f;
	int sectorCount = 36;
	int stackCount = 18;

	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float s, t;                                     // vertex texCoord

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			normals.push_back(nx);
			normals.push_back(ny);
			normals.push_back(nz);

			// vertex tex coord (s, t) range between [0, 1]
			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			texCoords.push_back(s);
			texCoords.push_back(t);
		}
	}

	// generate CCW index list of sphere triangles
// k1--k1+1
// |  / |
// | /  |
// k2--k2+1
	std::vector<int> indices;
	std::vector<int> lineIndices;
	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}

			// store indices for lines
			// vertical lines for all stacks, k1 => k2
			lineIndices.push_back(k1);
			lineIndices.push_back(k2);
			if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
			{
				lineIndices.push_back(k1);
				lineIndices.push_back(k1 + 1);
			}
		}
	}

	for (int i = 0; i < indices.size(); i++) {
		triangles.emplace_back(vertices[indices[i] * 3]);
		triangles.emplace_back(vertices[indices[i] * 3 + 1]);
		triangles.emplace_back(vertices[indices[i] * 3 + 2]);
	}

	return triangles;
}

GLsizei bindFaces(GLuint VA, GLuint VB)
{
	glBindVertexArray(VA);
	glBindBuffer(GL_ARRAY_BUFFER, VB);

	std::vector<GLfloat> vertices = generateSphereTriangles();
	std::vector<GLfloat> data;
	for (int i = 0; i < vertices.size(); i++)
		data.emplace_back(vertices[i]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), &data.front(), GL_STATIC_DRAW);

	// set vertex attribute pointers
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// unbind VB & VA
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return GLsizei(data.size());
}

void drawFaces()
{
	GLuint VA;
	GLuint VB;
	GLsizei nVert;
	glGenVertexArrays(1, &VA);
	glGenBuffers(1, &VB);
	nVert = bindFaces(VA, VB);

	Shader shader("face.vert.glsl", "face.frag.glsl");
	shader.Use();

	// get uniform locations
	GLint modelLoc = glGetUniformLocation(shader.Program, "model");
	GLint viewLoc = glGetUniformLocation(shader.Program, "view");
	GLint projLoc = glGetUniformLocation(shader.Program, "projection");

	// pass uniform values to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(*view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(*projection));

	// use the same color for all points
	GLint colorLoc = glGetUniformLocation(shader.Program, "ourColor");
	glUniform3fv(colorLoc, 1, glm::value_ptr(white));

	glBindVertexArray(VA);
	glDrawArrays(GL_TRIANGLES, 0, nVert);
	glBindVertexArray(0);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	GLFWwindow* window = glfwCreateWindow(800, 600, "Assignment2", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	// Initialize GLAD to setup the OpenGL Function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize glad" << std::endl;
		return -1;
	};

	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Create transformations
	init_model = std::make_unique<glm::mat4>(glm::rotate(glm::mat4(1.0f), glm::radians(00.0f), GLM_UP));
	//init_model = std::make_unique<glm::mat4>(1.0f);
	model = std::make_unique<glm::mat4>(*init_model);
	view = std::make_unique<glm::mat4>(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -30.0f)));
	projection = std::make_unique<glm::mat4>(
		glm::perspective(glm::radians(45.0f), (GLfloat)800.0 / (GLfloat)600.0, 0.1f, 1000.0f));


	Sphere sphere1(1, 36, 18, glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f)));
	Sphere sphere2(1, 36, 18, glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 0.0f)));
	Sphere sphere3(1, 36, 18, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
	Sphere sphere4(1, 36, 18, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f)));
	Sphere sphere5(1, 36, 18, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f)));
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Clear window
		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Draw
		*model = glm::rotate(*model, glm::radians(1.0f), glm::vec3(0.0f, 0.1f, 0.0f));
		//*model = glm::translate(*model, glm::vec3(0.1f, 0.0f, 0.0f));
		//drawFaces();
		sphere1.draw(*view, *projection);
		sphere2.draw(*view, *projection);
		sphere3.draw(*view, *projection);
		sphere4.draw(*view, *projection);
		sphere5.draw(*view, *projection);

		//Swap buffers
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}