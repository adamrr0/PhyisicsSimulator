#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#define GLEW_STATIC
#include <GL/glew.h>
#include <vector>

glm::vec3 sphericalToCartesian(float r, float theta, float phi);

class Circle {
public:
	float radius;
	float mass;
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 savedVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	GLuint sphereVAO = 0, sphereVBO = 0;
	int sphereVertexCount;

	void initSphereMesh();
	std::vector<float> sphereVertices();
	void accelerate(float ax, float ay, float az, float dt);
	void Position(float dt);
	void checkBounds(float dt);
	void Earthgravity(float dt);
	void collision(Circle& a, Circle& b, float elasticity);
	void drawSphere(GLuint shader, glm::mat4& view, glm::mat4 projection);


	Circle(glm::vec3 initPos, glm::vec3 initVel, glm::vec4 col, float r, float m)
	{
		this->position = initPos;
		this->velocity = initVel;
		this->color = col;
		this->radius = r;
		this->mass = m;
	}


private:
    static void ensure3(std::vector<float>& v) {
        if (v.size() < 3) v.resize(3, 0.0f);
    }


};