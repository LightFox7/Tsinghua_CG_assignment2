#include <glad/glad.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <SOIL.h>

#include "Shader.h"
#include "Sphere.h"

const float PI = acos(-1);

Sphere::Sphere(float radius, int sectorCount, int stackCount, std::shared_ptr<Sphere> focus,
	float distance, float startAngle, float startSpeed, std::string name, bool up, std::string texturePath)
	: radius(radius), sectorCount(sectorCount), stackCount(stackCount), focus(focus),
	distance(distance), angle(startAngle), speed(startSpeed), name(name), up(up), texturePath(texturePath)
{
	model = std::make_shared<glm::mat4>(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
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

	// Generate buffers
	glGenVertexArrays(1, &VA);
	glGenBuffers(1, &VB);

	glBindVertexArray(VA);
	glBindBuffer(GL_ARRAY_BUFFER, VB);

	std::vector<GLfloat>().swap(data);
	for (int i = 0; i < indices.size(); i++) {
		data.emplace_back(vertices[indices[i] * 3]);
		data.emplace_back(vertices[indices[i] * 3 + 1]);
		data.emplace_back(vertices[indices[i] * 3 + 2]);
		data.emplace_back(texCoords[indices[i] * 2]);
		data.emplace_back(texCoords[indices[i] * 2 + 1]);
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), &data.front(), GL_STATIC_DRAW);

	// set vertex attribute pointers
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// unbind VB & VA
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// End binding buffer
	nVert = data.size() / 5;

	// Load and create a texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load image, create texture and generate mipmaps
	int width, height;
	unsigned char* image = SOIL_load_image(texturePath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
}

// Update sphere position and rotation
void Sphere::update(float speedScale)
{
	*model = glm::rotate(*model, glm::radians(1.0f * speedScale), glm::vec3(0.0f, 0.0f, 1.0f));
	if (focus == nullptr)
		return;
	glm::mat4 focusModel = focus->getModel();
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(focusModel, scale, rotation, translation, skew, perspective);
	(*model)[3][0] = translation.x + distance * cos(angle * PI / 180.0);
	(*model)[3][1] = translation.y;
	(*model)[3][2] = translation.z + distance * sin(angle * PI / 180.0);
	// *model = glm::translate(glm::translate(glm::mat4(1.0f), translation),
	//	glm::vec3(distance * cos(angle * PI / 180.0), 0.0f, distance * sin(angle * PI / 180.0)));
	angle += speed * speedScale;
}

void Sphere::draw(glm::mat4 &view, glm::mat4 &projection)
{
	Shader shader("main.vert.glsl", "main.frag.glsl");
	shader.Use();

	// Drawing
	glBindTexture(GL_TEXTURE_2D, texture);
	// get uniform locations
	GLint modelLoc = glGetUniformLocation(shader.Program, "model");
	GLint viewLoc = glGetUniformLocation(shader.Program, "view");
	GLint projLoc = glGetUniformLocation(shader.Program, "projection");

	// pass uniform values to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(VA);
	glDrawArrays(GL_TRIANGLES, 0, nVert);
	glBindVertexArray(0);
}

void Sphere::drawText(glm::mat4& view, glm::mat4& projection, Text &text)
{
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(*model, scale, rotation, translation, skew, perspective);
	glm::vec4 clipSpacePos = projection * (view * glm::vec4(translation, 1.0));
	float x = 380 + (clipSpacePos.x / clipSpacePos.w) * 400;
	float y = 250;
	if (up)
		y += 80;
	text.Render(name, x, y, .3f, glm::vec3(.2f, .9f, .3f));
}