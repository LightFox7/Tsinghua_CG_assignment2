#include <iostream>
#include <memory>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>

#include <cmath>

// Other includes
#include "Shader.h"
#include "Sphere.h"
#include "Text.h"

bool displayNames = true;
bool displayHelp = true;
float speedScale = 1.0f;

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
		displayHelp = !displayHelp;
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
		displayNames = !displayNames;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS && speedScale < 2.0)
		speedScale += 0.1;
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS && speedScale > 0.0)
		speedScale -= 0.1;
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::unique_ptr<glm::mat4> view = nullptr;
	std::unique_ptr<glm::mat4> projection = nullptr;

	view = std::make_unique<glm::mat4>(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -50.0f)));
	projection = std::make_unique<glm::mat4>(
		glm::perspective(glm::radians(45.0f), (GLfloat)800.0 / (GLfloat)600.0, 0.1f, 1000.0f));

	// Init text display class
	Text text;

	const float distance = 3.0f;

	// Create planets
	std::shared_ptr<Sphere> sun = std::make_shared<Sphere>(2, 36, 18);
	std::shared_ptr<Sphere> mercury = std::make_shared<Sphere>(.2, 36, 18, sun, 1 * distance, 0.0f, 4.0f);
	std::shared_ptr<Sphere> venus = std::make_shared<Sphere>(.3, 36, 18, sun, 2 * distance, 0.0f, 1.8f);
	std::shared_ptr<Sphere> earth = std::make_shared<Sphere>(.5, 36, 18, sun, 3 * distance, 0.0f, 1.0f);
	std::shared_ptr<Sphere> moon = std::make_shared<Sphere>(.15, 36, 18, earth, .2 * distance, 0.0f, 2.0f);
	std::shared_ptr<Sphere> mars = std::make_shared<Sphere>(.25, 36, 18, sun, 4 * distance, 0.0f, 0.5f);
	std::shared_ptr<Sphere> jupyter = std::make_shared<Sphere>(1.2, 36, 18, sun, 5 * distance, 0.0f, 0.09f);
	std::shared_ptr<Sphere> saturn = std::make_shared<Sphere>(1.0, 36, 18, sun, 6 * distance, 0.0f, 0.03f);
	std::shared_ptr<Sphere> uranus = std::make_shared<Sphere>(.9, 36, 18, sun, 7 * distance, 0.0f, 0.01f);
	std::shared_ptr<Sphere> neptune = std::make_shared<Sphere>(.8, 36, 18, sun, 8 * distance, 0.0f, 0.005f);

	std::vector<std::shared_ptr<Sphere>> spheres;
	spheres.push_back(sun);
	spheres.push_back(mercury);
	spheres.push_back(venus);
	spheres.push_back(earth);
	spheres.push_back(moon);
	spheres.push_back(mars);
	spheres.push_back(jupyter);
	spheres.push_back(saturn);
	spheres.push_back(uranus);
	spheres.push_back(neptune);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Clear window
		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Draw
		for (auto it : spheres) {
			it->update(speedScale);
			it->draw(*view, *projection);
			if (displayNames)
				it->drawText(text);
		}
		if (displayHelp) {
			std::string speedtxt = std::to_string(speedScale);
			if (speedtxt.size() > 0 && speedtxt.at(0) == '-')
				speedtxt = speedtxt.substr(1, speedtxt.size());
			if (speedtxt.size() > 3)
				speedtxt = speedtxt.substr(0, 3);

			text.Render(std::string("Current speed: " + speedtxt).c_str(), 25.0f, 85.0f, 0.4f, glm::vec3(0.7, 0.7f, 0.2f));
			text.Render("Press <-/-> Arrow keys to speed up/down", 25.0f, 60.0f, 0.4f, glm::vec3(0.7, 0.7f, 0.2f));
			text.Render("Press N to toogle planet name display", 25.0f, 35.0f, 0.4f, glm::vec3(0.7, 0.7f, 0.2f));
			text.Render("Press H to toogle help display", 25.0f, 10.0f, 0.4f, glm::vec3(0.7, 0.7f, 0.2f));
		}

		//Swap buffers
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}