#define GLM_ENABLE_EXPERIMENTAL
#define GLEW_STATIC
#include <GL/glew.h>
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
#include "Shader.h"
#include "circle.h"
#include "camera.h"

Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));

// Create Circles
std::vector<Circle> circles = {
	// |Starting Position     X     Y      Z|  | Velocity  X      Y     Z|	 Color		R	   G	B	   A	  |Radius|   |Mass|
			Circle(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec4{0.0f, 0.0f, 1.0f , 1.0f},   1.0f    ,   5),		// BLUE
			Circle(glm::vec3{10.0f, 0.0f, 0.0f}, glm::vec3{-1.0f, 0.0f, 0.0f}, glm::vec4{0.0f, 1.0f, 0.0f , 0.0f},   0.15f   ,   5)			// GREEN 
};

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static bool first = true;
	static float lastX = 400, lastY = 300;

	if (first) { lastX = xpos; lastY = ypos; first = false; }

	float xoff = xpos - lastX;
	float yoff = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoff, yoff);
}

int main(void)
{
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 4.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float yaw = -90.0f;
	float pitch = 0.0f;
	float lastX = 400, lastY = 300;
	bool firstMouse = true;
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// Primary variables
	GLFWwindow* window;
	float G = 6.67430 * pow(10, -11); // Gravitational constant

	float screenwidth = 800;
	float screenheight = 600;

	int res = 100; // Resolution of circle
	float centerX = 0.0f;
	float centerY = 0.0f;
	float radius = 0.1f;

	// GUI variables
	float startTime = glfwGetTime();
	float pausedTime = 0.0f;
	float pauseStart = 0.0f;
	static bool pause = false;
	float lastTime = glfwGetTime();
	float simTime = 0.0f;
	float GlToMeters = 10.0f;

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

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW!" << std::endl;
		return -1;
	}
	glGetError();

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	for (auto& c : circles)
	{
		c.initSphereMesh();
	}

	// ImGUI Setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	GLuint programID = LoadShaders("Vertexshader.txt", "FragmentShader.txt");

	float aspect = screenwidth / screenheight;
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);


	while (!glfwWindowShouldClose(window))
	{
		// Time calculation
		float currentTime = glfwGetTime();
		float dt = currentTime - lastTime;
		float deltaTime = dt;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			camera.ProcessKeyboard(UP, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			camera.ProcessKeyboard(DOWN, deltaTime);

		// Random Colors
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	
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
		
		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(programID, "view"),
			1, GL_FALSE, glm::value_ptr(view));

		// Render Setup
		//glClear(GL_COLOR_BUFFER_BIT);
		//glColor3f(1.0f, 1.0f, 1.0f);

		//for (int i = 0; i <= 10; i++)
		//{
		//	int meters = i - 5;
		//	float x_lines = meters * 0.2;

		//	glBegin(GL_LINES);
		//	glVertex2f(x_lines, 1.0f);
		//	glVertex2f(x_lines, -1.0f);
		//	glEnd();
		//}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	
		// ImGUI Frame Start
		ImGui::Begin("Physics Simulator");
		if (ImGui::Button("Pause"))
		{
			pause = true;
			pauseStart = glfwGetTime();
			for (auto &c : circles)
			{
				c.savedVelocity = c.velocity;
				c.velocity = { 0.0f, 0.0f, 0.0f };
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
		ImGui::Text("Blue Ball Velocity (%.2f, %.2f) m/s", circles[0].velocity[0] * GlToMeters, circles[0].velocity[1] * GlToMeters);
		ImGui::Text("Green Ball Velocity (%.2f, %.2f) m/s", circles[1].velocity[0] * GlToMeters, circles[1].velocity[1] * GlToMeters);
		ImGui::Text("Blue Ball Position (%.2f, %.2f) m", circles[0].position[0] * GlToMeters, circles[0].position[1] * GlToMeters);
		ImGui::Text("Green Ball Position (%.2f, %.2f) m", circles[1].position[0] * GlToMeters, circles[1].position[1] * GlToMeters);
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
					float dz = b.position[2] - a.position[2];
					float distance = sqrt(dx * dx + dy * dy + dz * dz);

					if (distance > 0) {
						std::vector<float> direction = { dx / distance, dy / distance, dz / distance };
						float Gforce = G * (b.mass * a.mass) / (distance * distance);
						float ax = Gforce * direction[0] / a.mass;
						float ay = Gforce * direction[1] / a.mass;
						float az = Gforce * direction[2] / a.mass;
						std::vector<float>acc = { ax , ay , az};
						a.accelerate(acc[0], acc[1], acc[2], dt);
					}

					a.collision(a, b, 1.0f);	// 0.0f = inelastic, 1.0f = elastic
				//	a.Earthgravity(dt);
				}
				//a.checkBounds(dt);
				a.Position(dt);
			}
		}
		glUseProgram(programID);

		glUniform3f(glGetUniformLocation(programID, "lightPos"), 0.0f, 0.0f, 2.0f);
		glUniform3f(glGetUniformLocation(programID, "lightColor"), 1.0f, 1.0f, 1.0f);

		for (auto& a : circles)
		{
			a.drawSphere(programID, view, projection);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;

}
