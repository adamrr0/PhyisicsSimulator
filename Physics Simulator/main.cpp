#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include "CircleClass.h"
#include <iostream>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>
#include<imgui.h>
#include<backends/imgui_impl_glfw.h>
#include<backends/imgui_impl_opengl3.h>
#include "Camera.h"

class Circle {
public:
	float radius;
	float mass;
	std::vector<float> position = { 0.0f, 0.0f }; // x, y
	std::vector<float> velocity = { 0.0f, 0.0f }; // vx, vy
	std::vector<float> color = { 1.0f, 1.0f, 1.0f }; // r, g, b
	std::vector<float> savedVelocity = { 0.0f , 0.0f }; // For pause / unpause

	Circle(std::vector<float> pos, std::vector<float> vel, std::vector<float> col, float r, float m) : position(pos), velocity(vel), color(col), radius(r), mass(m)
	{
		this->position = pos;
		this->velocity = vel;
		this->color = col;
		this->radius = r;
		this->mass = m;
	}

	void accelerate(float ax, float ay, float dt)
	{
		this->velocity[0] += ax * dt;
		this->velocity[1] += ay * dt;
	}

	void Position(float dt)
	{
		this->position[0] += this->velocity[0] * dt;
		this->position[1] += this->velocity[1] * dt;

	}

	void drawCircle(float centerX, float centerY, float radius, int res)
	{
		glColor3f(color[0], color[1], color[2]);

		glBegin(GL_TRIANGLE_FAN);
		glVertex2d(centerX , centerY); // Center of circle

		for (int i = 0; i <= res; ++i)
		{
				float angle = 2.0f * 3.14159f * (static_cast<float>(i) / res); // Angle in radians
				float x = centerX + (radius * cos(angle));
				float y = centerY + (radius * sin(angle));
				glVertex2d(x, y);
		}
		glEnd();
	}

	void checkBounds(float dt)
	{

		if (position[0] + radius > 1.0f || position[0] - radius < -1.0f)
		{
			velocity[0] = -velocity[0];
			position[0] += velocity[0] * dt;
		}
		if(position[1] + radius > 0.75f || position[1] - radius < -0.75f)
		{
			velocity[1] = -velocity[1];
			position[1] += velocity[1] * dt;
		}
	}

	void Earthgravity(float dt)
	{
		const float g = -9.81f; 
		velocity[1] += g * dt;
	}

	void collision(Circle& c1, Circle& c2, float restitution)
	{
		float dx = c2.position[0] - c1.position[0];	// X distance between circles
		float dy = c2.position[1] - c1.position[1]; // Y distance between circles
		float distance = sqrt((dx * dx) + (dy * dy)); // Distance between circles

		float nx = dx / distance;	// Normalized X
		float ny = dy / distance;	// Normalized Y

		float rvx = c2.velocity[0] - c1.velocity[0];	// Relative X velocity
		float rvy = c2.velocity[1] - c1.velocity[1];	// Relative Y velocity

		float vrel = rvx * nx + rvy * ny;	// Relative velocity in terms of the normal direction


		float j = -(1 + restitution) * vrel / ((1 / c1.mass) + (1 / c2.mass));	// Impulse

		if (distance == 0) { return; }
		
		if (distance <= c1.radius + c2.radius)
		{
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

};



int main(void)
{
	// Primary variables
	GLFWwindow* window;
	float G = 6.67430 * pow(10, -11); // Gravitational constant

	float screenwidth = 800;
	float screenheight = 600;

	int res = 100; // Resolution of circle
	float centerX = 0.0f;
	float centerY = 0.0f;
	float radius = 0.1f;

	float startTime = glfwGetTime();
	float pausedTime = 0.0f;
	float pauseStart = 0.0f;
	static bool pause = false;
	float lastTime = glfwGetTime();
	float simTime = 0.0f;



	// Create Circles
	std::vector<Circle> circles = {
//			 |Starting Position     X     Y  |  |    Velocity       X     Y  |			Color		R	 G		B	   |Radius|   |      Mass     |
		Circle(std::vector<float>{0.0f, 0.0f}, std::vector<float>{0.00f, 0.0f}, std::vector<float>{0.0f, 0.0f, 1.0f},   0.05f    ,	1), // BLUE
		Circle(std::vector<float>{0.5f, 0.0f}, std::vector<float>{-0.75f, 0.0f}, std::vector<float>{0.0f, 1.0f, 0.0f},   0.05f  ,  1)// GREEN 
	};
	
	// Initialize GLFW
	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(screenwidth, screenheight, "Physics Simulator", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// ImGUI Setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// OpenGL Setup
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0f, 1.0f, -1.0f * (screenheight / screenwidth), 1.0f * (screenheight / screenwidth), -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);


	while (!glfwWindowShouldClose(window))
	{
		// Time calculation
		float currentTime = glfwGetTime();
		float dt = currentTime - lastTime;
	
		if (pause)
		{
			dt = 0.0f;
			lastTime = currentTime;
		}
		else
		{
			lastTime = currentTime;
			simTime += dt;
		}

		// Render Setup
		glClear(GL_COLOR_BUFFER_BIT);
		glColor3f(1.0f, 1.0f, 1.0f);
		
		glm::vec3 point = { -1.0f , 1.0f , 0.2f };
		std::vector<glm::vec3> points;
		float spacing = 0.05f;

		glPointSize(3.0f);

		for (int col = 0; col < 100; col++) {

			float x = -1.0f + col * spacing;

			for (int row = 0; row < 100; row++)
			{
				float y = -0.9f + row * spacing;
				points.push_back(glm::vec3(x, y, 0.0f));
			}
		}
		glBegin(GL_POINTS);

		for (auto dot : points)
		{
			glVertex2f(dot.x, dot.y);
		}
		glEnd();

		// ImGUI Frame Start
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Physics Simulator");
		if (ImGui::Button("Add Circle"))	// Button to add circles
		{
			float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

			circles.push_back(Circle(std::vector<float>{0.0f, 0.0f},
						   			std::vector<float>{0.0f, 0.0f},
									std::vector<float>{r, g, b},
													0.05f, 1));
		}
		if (ImGui::Button("Pause"))
		{
			pause = true;
			pauseStart = glfwGetTime();
			for (auto &c : circles)
			{
				c.savedVelocity = c.velocity;
				c.velocity = { 0.0f, 0.0f };
			}
		}
		if (ImGui::Button("Unpause"))
		{
			pause = false;
			pausedTime += glfwGetTime() - pauseStart;
			for (auto &c : circles)
			{
				c.velocity = c.savedVelocity;
			}
		}
		// Display circle information and gravity slider
		ImGui::Text("Blue Ball Velocity (%.4f, %.4f)", circles[0].savedVelocity[0], circles[0].savedVelocity[1]);
		ImGui::Text("Green Ball Velocity (%.2f, %.2f)", circles[1].savedVelocity[0], circles[1].savedVelocity[1]);
		ImGui::Text("Blue Ball Position (%.2f, %.2f)", circles[0].position[0], circles[0].position[1]);
		ImGui::Text("Green Ball Position (%.2f, %.2f)", circles[1].position[0], circles[1].position[1]);
		ImGui::Text("Time Elapsed: %.4f s", simTime);
		ImGui::End();


	
		// Physics and Drawing
		if (!pause) {
			simTime = glfwGetTime() - startTime - pausedTime;
				for (auto& a : circles) {
				for (auto& b : circles) {
					if (&a == &b) { continue; }

					float dx = b.position[0] - a.position[0];
					float dy = b.position[1] - a.position[1];
					float distance = sqrt(dx * dx + dy * dy);

					if (distance > 0) {
						std::vector<float> direction = { dx / distance, dy / distance };
						distance *= 1e20f;

						float Gforce = G * (b.mass * a.mass) / (distance * distance);
						float ax = Gforce * direction[0] / a.mass;
						float ay = Gforce * direction[1] / a.mass;
						std::vector<float>acc = { ax , ay };
						a.accelerate(acc[0], acc[1], dt);
					}

					a.collision(a, b, 0.9f);
					//a.Earthgravity(dt);
				}
				a.checkBounds(dt);
				a.Position(dt);
			}
		}

		for (auto& a : circles)
		{
			a.drawCircle(a.position[0], a.position[1], a.radius, res);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;

}
