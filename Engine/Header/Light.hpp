#pragma once

// OpenGL Includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// STD Includes
#include <string>

class Light
{
public:
	Light();
	glm::vec3 Color;
	float intensity;

private:

};

class PointLight : public Light
{
public:
	PointLight(){};

	std::string name;
private:
	glm::vec3 position;
	float radius;
};