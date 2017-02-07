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
#include "kiss_fft.h"

GLFWwindow* window;
const int width = 700;
const int height = 700;
const float g = 9.82f;
float stepSize;
float numOfVerts;
int numOfWaves = 20;
std::vector<vec2> waveDirections;
std::vector<vec2> waveHeights;
std::vector<kiss_fft_cpx> startHeights;
std::vector<kiss_fft_cpx> conjugatedStartHeights;
std::vector<vec2> vertexHeights;

void init();
void fillVectors(int size);
void fftFunc(vec2* output, kiss_fft_cpx cpx_in);
vec3 calculatePosAndNormal(int vertex, vec3 position);
void createWaveDirections(int amount, std::vector<vec2>* waves);
kiss_fft_cpx calculateStartHeight(vec2 dir, float randomR, float randomI, float amplitude, vec2 windDir);
void calculateWaveHeight(float waveNumber, float time);
float subdivide(int n, std::vector<vec3>* verts, std::vector<vec3>* norms, std::vector<vec2>* uvs);
void createQuad(std::vector<vec3>* verts, std::vector<vec3>* norms, std::vector<vec2>* uvs);


static const GLfloat g_vertex_buffer_quad[] = {
	5.0f, 0.0f, 5.0f,
	-5.0f, 0.0f, -5.0f,
	5.0f, 0.0f, -5.0f,
	5.0f, 0.0f, 5.0f,
	-5.0f, 0.0f, 5.0f,
	-5.0f, 0.0f, -5.0f,
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
	vec3 cameraPosition(0, 5, -10);
	vec3 cameraLookAtPosition(0, 0, 0);
	float nearPlane = 1;
	float farPlane = 25;
	vec3 lightPosition(0, 6, -6);

	init();

	//create vertex array
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	createWaveDirections(numOfWaves, &waveDirections);

	float amplitude = 3;
	float windSpeed = 0;
	vec2 windDir = vec2(-0.5, 0.5);
	//calculate the starting height for every wave (needs to b done once)
	for (int i = 0; i < waveDirections.size(); i++){
		float randomR = -8.8390e-2;
		float randomI = -7.8820e-1;
		startHeights.push_back(calculateStartHeight(waveDirections[i], randomR, randomI, amplitude, windDir));
		conjugatedStartHeights.push_back(calculateStartHeight(waveDirections[i], randomR, randomI, amplitude, windDir));
	}

	std::vector<vec3> quadVerts, quadNorms;
	std::vector<vec2> quadUVs;
	createQuad(&quadVerts, &quadNorms, &quadUVs);
	stepSize = subdivide(4, &quadVerts, &quadNorms, &quadUVs);
	numOfVerts = quadVerts.size();
	std::cout << "main num vert " << numOfVerts << std::endl;
	fillVectors(numOfVerts);

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

	GLuint vertexbuffer_vertex_heights;
	glGenBuffers(1, &vertexbuffer_vertex_heights);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_vertex_heights);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * numOfVerts, &(vertexHeights[0].x), GL_STATIC_DRAW);

	//Create and compile the shaders
	Shader demoShader;
	demoShader.createShader("vertexshader.glsl", "fragmentshader.glsl");
	GLuint demoMVPID = glGetUniformLocation(demoShader.programID, "MVP");
	GLuint demoLightID = glGetUniformLocation(demoShader.programID, "light");
	GLuint demoCameraID = glGetUniformLocation(demoShader.programID, "camera");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, nearPlane, farPlane);	
	glm::mat4 View = glm::lookAt(cameraPosition, cameraLookAtPosition, glm::vec3(0, 0, 1)); //Camera matrix, pos, look at, up
	glm::mat4 Model = glm::mat4(1.0f); //model transformation
	glm::mat4 mvp = Projection * View * Model;

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//main loop
	do {
		float currentTime = glfwGetTime();

		ImGui_ImplGlfwGL3_NewFrame();

		/*ImGui::Text("Parameters");
		ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 30.0f);
		ImGui::SliderFloat("Wind speed", &windSpeed, 0.0f, 50.0f);
		ImGui::SliderFloat("Wind xDir", &windDir.x, -1.0f, 1.0f);
		ImGui::SliderFloat("Wind yDir", &windDir.y, -1.0f, 1.0f);
		ImGui::SliderFloat("Wave xDir", &waveDirections[0].x, -10.0f, 10.0f);
		ImGui::SliderFloat("Wave yDir", &waveDirections[0].y, -10.0f, 10.0f);*/
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
		//calculate the height at this time step for every wave
		for (int wave = 0; wave < waveDirections.size(); wave++)
		{
			calculateWaveHeight(wave, currentTime);
		}

		//calculate the height at this time step for every vertex
		for (int vertex = 0; vertex < numOfVerts; vertex++)
		{
			quadNorms[vertex] = calculatePosAndNormal(vertex, quadVerts[vertex]);
		}
		//update buffers
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_vertex_heights);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * numOfVerts, &(vertexHeights[0].x), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_quad_normals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numOfVerts, &(quadNorms.at(0).x), GL_STATIC_DRAW);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(demoShader.programID);
		
		glUniformMatrix4fv(demoMVPID, 1, GL_FALSE, &mvp[0][0]);
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
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_vertex_heights);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDrawArrays(GL_TRIANGLES, 0, numOfVerts);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);

		ImGui::Render();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	ImGui_ImplGlfwGL3_Shutdown();
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer_quad);
	glDeleteBuffers(1, &vertexbuffer_quad_normals);
	glDeleteBuffers(1, &vertexbuffer_quad_tex);
	glDeleteBuffers(1, &vertexbuffer_vertex_heights);
	glDeleteProgram(demoShader.programID);
	glDeleteVertexArrays(1, &VertexArrayID);

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

void fillVectors(int size)
{
	for (int x = 0; x < size; x++)
	{
		vec2 temp(0, 0);
		vertexHeights.push_back(temp);
	}
	for (int i = 0; i < numOfWaves; i++)
	{
		waveHeights.push_back(vec2(0, 0));
	}
}

void fftFunc(vec2* output, kiss_fft_cpx cpx_in){
	int nfft = 1;
	int is_inverse_fft = 1;
	kiss_fft_cpx cpx_out;
	kiss_fft_cfg cfg = kiss_fft_alloc(nfft, is_inverse_fft, 0, 0);
	kiss_fft(cfg, &cpx_in, &cpx_out);
	output->x = cpx_out.r;
	output->y = cpx_out.i;
	free(cfg);
}

void createWaveDirections(int amount, std::vector<vec2>* waves)
{
	for(int i = 0; i < amount; i++){
		vec2 waveDir;
		waveDir.x = rand() % 2000 / 1000.0f;
		waveDir.y = (rand() % 4000 - 2000) / 1000.0f; std::cout << "waveDir: " << waveDir.x << " " << waveDir.y << std::endl;
		waves->push_back(waveDir);
	}
}

kiss_fft_cpx calculateStartHeight(vec2 dir, float randomR, float randomI, float amplitude, vec2 windDir)
{
	vec2 h = vec2(randomR, randomI) * abs(dot(dir / (float)dir.length(), windDir / (float)windDir.length())) / (float)pow(dir.length(), 2) *
		(float)sqrt(amplitude * 0.5 * exp(-1 / pow(dir.length() * pow(windDir.length(), 2) / g, 2)));
	kiss_fft_cpx height;
	height.r = h.x;
	height.i = h.y;
	return height;
}

void calculateWaveHeight(float waveNumber, float time){
	float frequency = sqrt(g * waveDirections[waveNumber].length());

	kiss_fft_cpx startH = startHeights[waveNumber];
	kiss_fft_cpx conjugatedH = conjugatedStartHeights[waveNumber];

	kiss_fft_cpx cpx;
	cpx.r = (startH.r + conjugatedH.r) * cos(frequency * time) + (conjugatedH.r - startH.i) * sin(frequency * time);
	cpx.i = (startH.i + conjugatedH.i) * cos(frequency * time) + (startH.r - conjugatedH.r) * sin(frequency * time);

	waveHeights[waveNumber].x = cpx.r;
	waveHeights[waveNumber].y = cpx.i;
}

vec3 calculatePosAndNormal(int vertex, vec3 vertexposition)
{
	vec2 pos = vec2(vertexposition.x, vertexposition.z);
	vec2 height = vec2(0, 0);
	vec2 slope_x(0, 0);
	vec2 slope_y(0, 0);

	for (int i = 0; i < numOfWaves; i++){
		vec2 wh = waveHeights[i];
		//The dot product tells you what amount of one vector goes in the direction of another
		float dotTerm = dot(waveDirections[i], pos);
		float cosTerm = cos(dotTerm);
		float sinTerm = sin(dotTerm);
		float xTerm = wh.x * cosTerm - wh.y * sinTerm;
		float yTerm = wh.y * cosTerm + wh.x * sinTerm;
		height += vec2(xTerm, yTerm);
		slope_x += vec2(-(waveDirections[i].x * yTerm), waveDirections[i].x * xTerm);
		slope_y += vec2(-(waveDirections[i].y * yTerm), waveDirections[i].y * xTerm);
	}

	kiss_fft_cpx cpx;
	cpx.r = height.x;
	cpx.i = height.y;
	fftFunc(&vertexHeights[vertex], cpx);

	kiss_fft_cpx cpx_x;
	kiss_fft_cpx cpx_y;
	cpx_x.r = slope_x.x;
	cpx_x.i = slope_x.y;
	cpx_y.r = slope_y.x;
	cpx_y.i = slope_y.y;

	vec2 fftx, ffty;
	fftFunc(&fftx, cpx_x);
	fftFunc(&ffty, cpx_y);

	vec2 deltaH(fftx.x, ffty.x);
	vec3 normalx(stepSize, deltaH.x, 0);
	vec3 normalz(0, deltaH.y, stepSize);
	return normalize(cross(normalx, normalz));
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
