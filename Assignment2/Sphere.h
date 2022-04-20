#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

class Sphere
{
public:
	// Ctor / Dtor
	Sphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18, glm::mat4 modelParam = glm::mat4(1.0f));
	~Sphere();

	// Generate sphere
	void Generate();

	// Draw sphere
	void draw(glm::mat4& view, glm::mat4& projection);
protected:
	// Parameters
	float radius;
	int sectorCount;
	int stackCount;

	// Position and rotation
	std::shared_ptr<glm::mat4> model;

	// Drawing info
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> texCoords;
	std::vector<int> indices;
	std::vector<int> lineIndices;
};

