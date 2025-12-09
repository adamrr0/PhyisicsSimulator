#define GLM_ENABLE_EXPERIMENTAL
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>
#include<imgui.h>
#include<backends/imgui_impl_glfw.h>
#include<backends/imgui_impl_opengl3.h>
#include "circle.h"

glm::vec3 sphericalToCartesian(float r, float theta, float phi)
{
	float x = r * sin(theta) * cos(phi);
	float y = r * sin(theta) * sin(phi);
	float z = r * cos(theta);
	return glm::vec3(x, y, z);
}

	std::vector<float> Circle::sphereVertices(int stacks, int sectors) const
	{
		std::vector<float> vertices;

		for (int i = 0; i <= stacks; i++)
		{
			float theta1 = (float)i / stacks * glm::pi<float>();
			float theta2 = (float)(i + 1) / stacks * glm::pi<float>();

			for (int j = 0; j < sectors; j++)
			{
				float phi1 = (float)j / sectors * 2.0f * glm::pi<float>();
				float phi2 = (float)(j + 1) / sectors * 2.0f * glm::pi<float>();

				glm::vec3 v1 = sphericalToCartesian(radius, theta1, phi1);
				glm::vec3 v2 = sphericalToCartesian(radius, theta2, phi1);
				glm::vec3 v3 = sphericalToCartesian(radius, theta2, phi2);
				glm::vec3 v4 = sphericalToCartesian(radius, theta1, phi2);

				glm::vec3 n1 = glm::normalize(v1);
				glm::vec3 n2 = glm::normalize(v2);
				glm::vec3 n3 = glm::normalize(v3);
				glm::vec3 n4 = glm::normalize(v4);

				vertices.insert(vertices.end(), { v1.x, v1.y, v1.z, n1.x, n1.y, n1.z });
				vertices.insert(vertices.end(), { v2.x, v2.y, v2.z, n2.x, n2.y, n2.z });
				vertices.insert(vertices.end(), { v3.x, v3.y, v3.z, n3.x, n3.y, n3.z });

				vertices.insert(vertices.end(), { v2.x, v2.y, v2.z, n2.x, n2.y, n2.z });
				vertices.insert(vertices.end(), { v4.x, v4.y, v4.z, n4.x, n4.y, n4.z });
				vertices.insert(vertices.end(), { v3.x, v3.y, v3.z, n3.x, n3.y, n3.z });
			}
		}
		return vertices;
	}

	void Circle::initSphereMesh()
	{
		std::vector<float> verts = sphereVertices();
		sphereVertexCount = verts.size() / 6;

		glGenVertexArrays(1, &sphereVAO);
		glGenBuffers(1, &sphereVBO);

		glBindVertexArray(sphereVAO);

		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}

	void Circle::drawSphere(GLuint shaderID, glm::mat4& view, glm::mat4 projection)
	{
		glUseProgram(shaderID);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position[0], position[1], 0.0f));

		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform3f(glGetUniformLocation(shaderID, "sphereColor"), color[0], color[1], color[2]);

		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
		glBindVertexArray(0);
	}


	void Circle::accelerate(float ax, float ay, float az, float dt)
	{
		this->velocity[0] += ax * dt;
		this->velocity[1] += ay * dt;
		this->velocity[2] += az * dt;
	}

	void Circle::Position(float dt)
	{
		this->position[0] += this->velocity[0] * dt;
		this->position[1] += this->velocity[1] * dt;
		this->position[2] += this->velocity[2] * dt;
	}

	//void Circle::checkBounds(float dt)
	//{

	//	if (position[0] + radius > 1.0f || position[0] - radius < -1.0f)
	//	{
	//		velocity[0] = -velocity[0];
	//		position[0] += velocity[0] * dt;
	//	}
	//	if (position[1] + radius > 1.0f || position[1] - radius < -1.0f)
	//	{
	//		velocity[1] = -velocity[1];
	//		position[1] += velocity[1] * dt;
	//	}
	//	if (position[2] + radius > 1.0f || position[2] - radius < -1.0f)
	//	{
	//		velocity[2] = -velocity[2];
	//		position[2] += velocity[2] * dt;
	//	}
	//}

	void Circle::Earthgravity(float dt)
	{
		const float g = -9.81f;
		velocity[1] += g * dt;
	}

	void Circle::collision(Circle& c1, Circle& c2, float restitution)
	{
		float dx = c2.position[0] - c1.position[0];	// X distance between circles
		float dy = c2.position[1] - c1.position[1]; // Y distance between circles
		float distance = sqrt((dx * dx) + (dy * dy)); // Distance between circles

		if (distance == 0) { return; }

		if (distance <= c1.radius + c2.radius)
		{
			float nx = dx / distance;	// Normalized X
			float ny = dy / distance;	// Normalized Y

			float rvx = c2.velocity[0] - c1.velocity[0];	// Relative X velocity
			float rvy = c2.velocity[1] - c1.velocity[1];	// Relative Y velocity

			float vrel = rvx * nx + rvy * ny;	// Relative velocity in terms of the normal direction

			float j = -(1 + restitution) * vrel / ((1 / c1.mass) + (1 / c2.mass));	// Impulse

			c1.velocity[0] -= (j / c1.mass) * nx;
			c1.velocity[1] -= (j / c1.mass) * ny;
			c2.velocity[0] += (j / c2.mass) * nx;
			c2.velocity[1] += (j / c2.mass) * ny;

			float overlap = (c1.radius + c2.radius) - distance;
			c1.position[0] -= (overlap / 2) * nx;
			c1.position[1] -= (overlap / 2) * ny;
			c2.position[0] += (overlap / 2) * nx;
			c2.position[1] += (overlap / 2) * ny;

		}
	}