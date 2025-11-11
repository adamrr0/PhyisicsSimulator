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
	float dt = 0.016f; // Assuming a fixed time step for simplicity
	float radius;
	float mass;
	std::vector<float> position = { 0.0f, 0.0f }; // x, y
	std::vector<float> velocity = { 0.0f, 0.0f }; // vx, vy
	std::vector<float> color = { 1.0f, 1.0f, 1.0f }; // r, g, b

	Circle(std::vector<float> pos, std::vector<float> vel, std::vector<float> col, float r, float m) : position(pos), velocity(vel), color(col), radius(r), mass(m)
	{
		this->position = pos;
		this->velocity = vel;
		this->color = col;
		this->radius = r;
		this->mass = m;
	}

	void accelerate(float ax, float ay)
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
	float lastTime = glfwGetTime();

	// Create Circles
	std::vector<Circle> circles = {
//			 |Starting Position     X     Y  |  |    Velocity       X     Y  |			Color		R	 G		B	   |Radius|   |      Mass     |
		Circle(std::vector<float>{0.5f, 0.5f}, std::vector<float>{0.5f, 0.5f}, std::vector<float>{0.0f, 0.0f, 1.0f},   0.05f    ,	1), // BLUE
		Circle(std::vector<float>{-0.5f, -0.5f}, std::vector<float>{0.2f, 0.0f} , std::vector<float>{0.0f, 1.0f, 0.0f},   0.05f    ,  	1)// GREEN 
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
		// Render Setup
		glClear(GL_COLOR_BUFFER_BIT);

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
		// Display circle information and gravity slider
		ImGui::Text("Blue Ball Velocity (%.4f, %.4f)", circles[0].velocity[0], circles[0].velocity[1]);
		ImGui::Text("Green Ball Velocity (%.2f, %.2f)", circles[1].velocity[0], circles[1].velocity[1]);
		ImGui::Text("Blue Ball Position (%.2f, %.2f)", circles[0].position[0], circles[0].position[1]);
		ImGui::Text("Green Ball Position (%.2f, %.2f)", circles[1].position[0], circles[1].position[1]);
		ImGui::SliderAngle("Gravity", &G, -10.0f, 10.0f);
		ImGui::End();

		// Time calculation
		float currentTime = glfwGetTime();
		float dt = currentTime - lastTime;
		lastTime = currentTime;
	
		// Physics and Drawing
		for (auto& a: circles){
			for (auto& b: circles){
				if (&a == &b) { continue; }
				
				float dx = b.position[0] - a.position[0];
				float dy = b.position[1] - a.position[1];
				float distance = sqrt(dx * dx + dy * dy);

				//if (distance > 0) {
				//	std::vector<float> direction = { dx / distance, dy / distance };
				//	distance *= 1e10f;

				//	float Gforce = G * (b.mass * a.mass) / (distance * distance);
				//	float ax = Gforce * direction[0] / a.mass;
				//	float ay = Gforce * direction[1] / a.mass;
				//	std::vector<float>acc = { ax , ay };
				//	a.accelerate(acc[0], acc[1]);
				//}

				a.collision(a, b, 0.9f);
				a.Earthgravity(dt);
				}
			a.checkBounds(dt);
			a.drawCircle(a.position[0], a.position[1], a.radius, res);
			a.Position(dt);
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
