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
	std::vector<float> position;
	std::vector<float> velocity;
	std::vector<float> color;
	std::vector<float> savedVelocity;

	GLuint sphereVAO = 0, sphereVBO = 0;
	int sphereVertexCount;

	void initSphereMesh();
	std::vector<float> sphereVertices(int stacks = 30, int sectors = 30) const;
	void accelerate(float ax, float ay, float az, float dt);
	void Position(float dt);
	void checkBounds(float dt);
	void Earthgravity(float dt);
	void collision(Circle& a, Circle& b, float elasticity);
	void drawSphere(GLuint shader, glm::mat4& view, glm::mat4 projection);

	    Circle(std::vector<float> pos, std::vector<float> vel, std::vector<float> col, float r, float m)
        : radius(r), mass(m), position(std::move(pos)), velocity(std::move(vel)), color(std::move(col))
    {
        ensure3(position);
        ensure3(velocity);
        ensure3(color);
        ensure3(savedVelocity); 
	}

private:
    static void ensure3(std::vector<float>& v) {
        if (v.size() < 3) v.resize(3, 0.0f);
    }


};