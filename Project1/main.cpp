#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <GL/glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <imgui.h>
#include <imgui_internal.h>
#include <stb_rect_pack.h>
#include <stb_textedit.h>
#include <stb_truetype.h>
using namespace glm;

#include "myShader.h"
#include "imgui_impl_glfw_gl3.h"

GLFWwindow* window;
const int width = 700;
const int height = 700;

void init(); 
float subdivide(int n, std::vector<vec3>* verts, std::vector<vec3>* norms, std::vector<vec2>* uvs);
void createQuad(std::vector<vec3>* verts, std::vector<vec3>* norms, std::vector<vec2>* uvs);


static const GLfloat g_vertex_buffer_quad[] = {
	1.0f, 0.0f, 1.0f,
	-1.0f, 0.0f, -1.0f,
	1.0f, 0.0f, -1.0f,
	1.0f, 0.0f, 1.0f,
	-1.0f, 0.0f, 1.0f,
	-1.0f, 0.0f, -1.0f,
};

static const GLfloat g_vertex_buffer_quad_normals[] = {
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
};

static const GLfloat g_vertex_buffer_quad_tex[] = {
	1.0f, 1.0f, 
	0.0f, 0.0f, 
	1.0f, 0.0f, 
	1.0f, 1.0f,
	0.0f, 1.0f, 
	0.0f, 0.0f,
};

int main(){
	vec3 cameraPosition(0, 4, -8);
	vec3 cameraLookAtPosition(0, 0, 0);
	float nearPlane = 1;
	float farPlane = 10;
	vec3 lightPosition(2, 3, 0);

	init();

	//FPS
		double lastTime = glfwGetTime();
		int nbFrames = 0;

	//create vertex array
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	std::vector<vec3> quadVerts, quadNorms;
	std::vector<vec2> quadUVs;
	createQuad(&quadVerts, &quadNorms, &quadUVs);
	float stepSize = subdivide(6, &quadVerts, &quadNorms, &quadUVs);
	int numOfVerts = quadVerts.size();
	std::cout << "main num vert " << numOfVerts << std::endl;

	//Load grid into buffer
	GLuint vertexbuffer_quad;
	glGenBuffers(1, &vertexbuffer_quad);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numOfVerts, &(quadVerts.at(0).x), GL_STATIC_DRAW);

	GLuint vertexbuffer_quad_normals;
	glGenBuffers(1, &vertexbuffer_quad_normals);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_quad_normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numOfVerts, &(quadNorms.at(0).x), GL_STATIC_DRAW);

	GLuint vertexbuffer_quad_tex;
	glGenBuffers(1, &vertexbuffer_quad_tex);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_quad_tex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * numOfVerts, &(quadUVs.at(0).x), GL_STATIC_DRAW);

	//for textures
	//create framebuffer
	/*GLuint framebuffer = 0;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;*/

	//Create and compile the shaders
	Shader createTextureShader;
	createTextureShader.createShader("vertOnlyMoves.glsl", "fragGiveColor.glsl");
	GLuint MatrixIDDepthTexture = glGetUniformLocation(createTextureShader.programID, "MVP");

	Shader demoShader;
	demoShader.createShader("vertexshader.glsl", "fragmentshader.glsl");
	GLuint demoMVID = glGetUniformLocation(demoShader.programID, "MV");
	GLuint demoPID = glGetUniformLocation(demoShader.programID, "P");
	GLuint demoTimeID = glGetUniformLocation(demoShader.programID, "time");
	GLuint demoLightID = glGetUniformLocation(demoShader.programID, "light");
	GLuint demoCameraID = glGetUniformLocation(demoShader.programID, "camera");
	GLuint demoStepID = glGetUniformLocation(demoShader.programID, "stepSize");
	
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, nearPlane, farPlane);	
	glm::mat4 View = glm::lookAt(cameraPosition, cameraLookAtPosition, glm::vec3(0, 0, 1)); //Camera matrix, pos, look at, up
	glm::mat4 Model = glm::mat4(1.0f); //model transformation
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 mv = /*Projection */ View * Model;
	glm::mat4 Light = glm::lookAt(lightPosition, cameraLookAtPosition, glm::vec3(0, 0, 1));
	glm::mat4 mvpLight = Projection * Light * Model;

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//main loop
	do {
		// FPS
			/*double currentTime = glfwGetTime();
			nbFrames++;
			if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1 sec ago
				// printf and reset timer
				printf("%i frames/s\n", nbFrames);
				nbFrames = 0;
				lastTime += 1.0;
			}*/

		ImGui_ImplGlfwGL3_NewFrame();

		// 1. Show a simple window
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			static float f = 0.0f;
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			//ImGui::ColorEdit3("clear color", (float*)&clear_color);
			//if (ImGui::Button("Test Window")) show_test_window ^= 1;
			//if (ImGui::Button("Another Window")) show_another_window ^= 1;
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		// 2. Show another simple window, this time using an explicit Begin/End pair
		/*if (show_another_window)
		{
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Another Window", &show_another_window);
			ImGui::Text("Hello");
			ImGui::End();
		}

		// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
		if (show_test_window)
		{
			ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
			ImGui::ShowTestWindow(&show_test_window);
		}*/

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(demoShader.programID);
		
		glUniformMatrix4fv(demoMVID, 1, GL_FALSE, &mv[0][0]);
		glUniformMatrix4fv(demoPID, 1, GL_FALSE, &Projection[0][0]);
		glUniform1f(demoTimeID, glfwGetTime());
		glUniform1f(demoStepID, stepSize);
		glUniform3f(demoLightID, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(demoCameraID, cameraPosition.x, cameraPosition.y, cameraPosition.z);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_quad);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_quad_normals);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_quad_tex);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDrawArrays(GL_TRIANGLES, 0, numOfVerts);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		ImGui::Render();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	ImGui_ImplGlfwGL3_Shutdown();
	// Cleanup VBO and shader
	/*glDeleteBuffers(1, &vertexbuffer);
	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteProgram(createTextureShader.programID);
	glDeleteProgram(intersectionShader.programID);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteTextures(1, &depthTexture);
*/
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

void init(){
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		char a;
		std::cin >> a;
		exit(-1);
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height, "Tutorial", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		char a;
		std::cin >> a;
		exit(-1);
	}

	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		char a;
		std::cin >> a;
		exit(-1);
	}

	glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	ImGui_ImplGlfwGL3_Init(window, true);
}

float subdivide(int n, std::vector<vec3>* verts, std::vector<vec3>* norms, std::vector<vec2>* uvs)
{
	std::vector<vec3> newVerts;
	std::vector<vec3> newNorms;
	std::vector<vec2> newUVs;
	for (int j = 0; j < n; j++)
	{
		for (int i = 0; i < verts->size() / 3; i++)
		{
			vec3 pos1 = verts->at(i * 3) + (verts->at(i * 3 + 1) - verts->at(i * 3)) / 2.0f;
			vec3 pos2 = verts->at(i * 3 + 1) + (verts->at(i * 3 + 2) - verts->at(i * 3 + 1)) / 2.0f;
			vec3 pos3 = verts->at(i * 3 + 2) + (verts->at(i * 3) - verts->at(i * 3 + 2)) / 2.0f;

			vec3 n1 = normalize(norms->at(i * 3) + norms->at(i * 3));
			vec3 n2 = normalize(norms->at(i * 3 + 1) + norms->at(i * 3 + 1));
			vec3 n3 = normalize(norms->at(i * 3 + 2) + norms->at(i * 3 + 2));

			vec2 uv1 = uvs->at(i * 3) + (uvs->at(i * 3 + 1) - uvs->at(i * 3)) / 2.0f;
			vec2 uv2 = uvs->at(i * 3 + 1) + (uvs->at(i * 3 + 2) - uvs->at(i * 3 + 1)) / 2.0f;
			vec2 uv3 = uvs->at(i * 3 + 2) + (uvs->at(i * 3) - uvs->at(i * 3 + 2)) / 2.0f;

			newVerts.push_back(verts->at(i * 3));
			newVerts.push_back(pos1);
			newVerts.push_back(pos3);
			newNorms.push_back(norms->at(i * 3));
			newNorms.push_back(n1);
			newNorms.push_back(n3);
			newUVs.push_back(uvs->at(i * 3));
			newUVs.push_back(uv1);
			newUVs.push_back(uv3);

			newVerts.push_back(pos1);
			newVerts.push_back(verts->at(i * 3 + 1));
			newVerts.push_back(pos2);
			newNorms.push_back(n1);
			newNorms.push_back(norms->at(i * 3 + 1));
			newNorms.push_back(n2);
			newUVs.push_back(uv1);
			newUVs.push_back(uvs->at(i * 3 + 1));
			newUVs.push_back(uv2);

			newVerts.push_back(pos1);
			newVerts.push_back(pos2);
			newVerts.push_back(pos3);
			newNorms.push_back(n1);
			newNorms.push_back(n2);
			newNorms.push_back(n3);
			newUVs.push_back(uv1);
			newUVs.push_back(uv2);
			newUVs.push_back(uv3);

			newVerts.push_back(pos3);
			newVerts.push_back(pos2);
			newVerts.push_back(verts->at(i * 3 + 2));
			newNorms.push_back(n3);
			newNorms.push_back(n2);
			newNorms.push_back(norms->at(i * 3 + 2));
			newUVs.push_back(uv3);
			newUVs.push_back(uv2);
			newUVs.push_back(uvs->at(i * 3 + 2));
		}

		*verts = std::move(newVerts);
		*norms = std::move(newNorms);
		*uvs = std::move(newUVs);
	}

	return abs(verts->at(0).x - verts->at(1).x);
}

void createQuad(std::vector<vec3>* verts, std::vector<vec3>* norms, std::vector<vec2>* uvs){
	
	verts->push_back(vec3(g_vertex_buffer_quad[0], g_vertex_buffer_quad[1], g_vertex_buffer_quad[2]));
	verts->push_back(vec3(g_vertex_buffer_quad[3], g_vertex_buffer_quad[4], g_vertex_buffer_quad[5]));
	verts->push_back(vec3(g_vertex_buffer_quad[6], g_vertex_buffer_quad[7], g_vertex_buffer_quad[8]));

	norms->push_back(vec3(g_vertex_buffer_quad_normals[0], g_vertex_buffer_quad_normals[1], g_vertex_buffer_quad_normals[2]));
	norms->push_back(vec3(g_vertex_buffer_quad_normals[3], g_vertex_buffer_quad_normals[4], g_vertex_buffer_quad_normals[5]));
	norms->push_back(vec3(g_vertex_buffer_quad_normals[6], g_vertex_buffer_quad_normals[7], g_vertex_buffer_quad_normals[8]));

	uvs->push_back(vec2(g_vertex_buffer_quad_tex[0], g_vertex_buffer_quad_tex[1]));
	uvs->push_back(vec2(g_vertex_buffer_quad_tex[2], g_vertex_buffer_quad_tex[3]));
	uvs->push_back(vec2(g_vertex_buffer_quad_tex[4], g_vertex_buffer_quad_tex[5]));

	verts->push_back(vec3(g_vertex_buffer_quad[9], g_vertex_buffer_quad[10], g_vertex_buffer_quad[11]));
	verts->push_back(vec3(g_vertex_buffer_quad[12], g_vertex_buffer_quad[13], g_vertex_buffer_quad[14]));
	verts->push_back(vec3(g_vertex_buffer_quad[15], g_vertex_buffer_quad[16], g_vertex_buffer_quad[17]));

	norms->push_back(vec3(g_vertex_buffer_quad_normals[9], g_vertex_buffer_quad_normals[10], g_vertex_buffer_quad_normals[11]));
	norms->push_back(vec3(g_vertex_buffer_quad_normals[12], g_vertex_buffer_quad_normals[13], g_vertex_buffer_quad_normals[14]));
	norms->push_back(vec3(g_vertex_buffer_quad_normals[15], g_vertex_buffer_quad_normals[16], g_vertex_buffer_quad_normals[17]));

	uvs->push_back(vec2(g_vertex_buffer_quad_tex[6], g_vertex_buffer_quad_tex[7]));
	uvs->push_back(vec2(g_vertex_buffer_quad_tex[8], g_vertex_buffer_quad_tex[9]));
	uvs->push_back(vec2(g_vertex_buffer_quad_tex[10], g_vertex_buffer_quad_tex[11]));
}