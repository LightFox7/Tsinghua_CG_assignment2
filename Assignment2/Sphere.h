#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include "Text.h"

class Sphere
{
public:
	// Ctor / Dtor
	Sphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18, std::shared_ptr<Sphere> focus = nullptr,
		float distance = 0.0f, float startAngle = 0.0f, float startSpeed = 0.0f, std::string name = "planet", bool up = true);
	~Sphere();

	// Getters
	glm::mat4 getModel() { return *model; };

	// Update
	void update(float speedScale = 1.0f);
	// Draw sphere
	void draw(glm::mat4& view, glm::mat4& projection);
	void drawText(glm::mat4& view, glm::mat4& projection, Text& text);
protected:
	// Generate sphere
	void Generate();
	// Parameters
	float radius;
	int sectorCount;
	int stackCount;
	std::string name;
	bool up;

	// Focus sphere param
	std::shared_ptr<Sphere> focus;
	float angle; // angle relative
	float speed; // speed in degrees per frame
	float distance;

	// Position and rotation
	std::shared_ptr<glm::mat4> model;

	// Drawing info
	GLuint VA;
	GLuint VB;
	GLsizei nVert;
	std::vector<GLfloat> data;
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> texCoords;
	std::vector<int> indices;
	std::vector<int> lineIndices;
};

