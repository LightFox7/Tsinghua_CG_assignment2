#include <glad/glad.h>

#include "Shader.h"
#include "Sphere.h"

const float PI = acos(-1);

Sphere::Sphere(float radius, int sectorCount, int stackCount, glm::mat4 modelParam)
	: radius(radius), sectorCount(sectorCount), stackCount(stackCount)
{
	model = std::make_shared<glm::mat4>(modelParam);
	Generate();
}

Sphere::~Sphere()
{

}

void Sphere::Generate()
{
	std::vector<float>().swap(vertices);
	std::vector<float>().swap(normals);
	std::vector<float>().swap(texCoords);
	std::vector<int>().swap(indices);
	std::vector<int>().swap(lineIndices);

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

	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);
		k2 = k1 + sectorCount + 1;

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}

			lineIndices.push_back(k1);
			lineIndices.push_back(k2);
			if (i != 0)
			{
				lineIndices.push_back(k1);
				lineIndices.push_back(k1 + 1);
			}
		}
	}
}

void Sphere::draw(glm::mat4 &view, glm::mat4 &projection)
{
	// Generate buffers
	GLuint VA;
	GLuint VB;
	GLsizei nVert;
	std::vector<GLfloat> data;

	glGenVertexArrays(1, &VA);
	glGenBuffers(1, &VB);

	glBindVertexArray(VA);
	glBindBuffer(GL_ARRAY_BUFFER, VB);

	for (int i = 0; i < indices.size(); i++) {
		data.emplace_back(vertices[indices[i] * 3]);
		data.emplace_back(vertices[indices[i] * 3 + 1]);
		data.emplace_back(vertices[indices[i] * 3 + 2]);
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), &data.front(), GL_STATIC_DRAW);

	// set vertex attribute pointers
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// unbind VB & VA
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// End binding buffer
	nVert = data.size() / 3;

	Shader shader("face.vert.glsl", "face.frag.glsl");
	shader.Use();

	// Drawing
	// get uniform locations
	GLint modelLoc = glGetUniformLocation(shader.Program, "model");
	GLint viewLoc = glGetUniformLocation(shader.Program, "view");
	GLint projLoc = glGetUniformLocation(shader.Program, "projection");

	// pass uniform values to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// use the same color for all points
	GLint colorLoc = glGetUniformLocation(shader.Program, "ourColor");
	glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(1, 1, 1)));

	glBindVertexArray(VA);
	glDrawArrays(GL_TRIANGLES, 0, nVert);
	glBindVertexArray(0);
}