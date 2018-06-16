#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor, programTexture, programTextureSpecular, programPerlin, programVoro, programFadedStripes, programSharpStripes;
obj::Model sand, ruins, sharkModel, fishModel, f2, f3, f4, f5, bubble, plant, reef;
GLuint  texBubble, texFish;
int cameraVelocityINT = 3;
bool isManual = false, isPaused = false;

Core::Shader_Loader shaderLoader;
float appLoadingTime, appLoadingTime_forShaders;


float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(5.0f, -5.9f, -5.0f));


static const int NUM_FISHSPOTS = 12;
static const int NUM_BUBBLES = 500;
static const int NUM_CAMERA_POINTS = 14;
static const int MOVE_POINTS = 10;

glm::vec3 bubbleStartPoints[NUM_BUBBLES];
float bubbleSizes[NUM_BUBBLES];
glm::vec3 fishStartPositions[NUM_FISHSPOTS];
glm::vec3 cameraKeyPoints[NUM_CAMERA_POINTS];
glm::vec3 fishMovementPoints[MOVE_POINTS];
glm::vec3 sharkMovementPoints[MOVE_POINTS];

int bubbleSwitchAlpha = 0;
int bubbleSwitchBeta = 0;

void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.2f;
	float moveSpeed = 0.3f;
	switch (key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed; break;


	case 'r': cameraPos -= glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeed; break;
	case 'f': cameraPos += glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeed; break;

	case '+': if (cameraVelocityINT > 1) cameraVelocityINT--; break;
	case '-': if (cameraVelocityINT < 5) cameraVelocityINT++; break;

	case 'm': isManual == false ? isManual = true : isManual = false; break;

	case 'p': isPaused == false ? isPaused = true : isPaused = false; break;
	}
}

glm::mat4 createBubbleMovementMatrixAlpha() {

	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - appLoadingTime;
	glm::mat4 movementTranslation;
	movementTranslation[3] = glm::vec4(0, time - bubbleSwitchAlpha, 0, 1);

	if (movementTranslation[3].y > 30) { bubbleSwitchAlpha += 60; }
	movementTranslation[3][1] = time - bubbleSwitchAlpha;

	return movementTranslation;
}

glm::mat4 createBubbleMovementMatrixBeta() {

	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - appLoadingTime;
	glm::mat4 movementTranslation;
	movementTranslation[3] = glm::vec4(0, time - bubbleSwitchBeta, 0, 1);

	if (movementTranslation[3].y > 60) { bubbleSwitchBeta += 60; }
	movementTranslation[3] = glm::vec4(0, time - bubbleSwitchBeta, 0, 1);

	return movementTranslation;
}

// 
glm::mat4 createFishMovementMatrix(glm::vec3 points[], float speed) {

	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - appLoadingTime;

	if (isPaused == false) {
	}

	if (isPaused == true) {
		static float stopTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - appLoadingTime;
		appLoadingTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - stopTime;
	}

	glm::vec3 up = glm::vec3(0, 1, 0);
	float s = glm::fract(time / speed);
	int xx = floorf(time / speed);

	glm::vec3 forward = glm::normalize(glm::catmullRom(
		points[(xx - 1) % MOVE_POINTS],
		points[(xx) % MOVE_POINTS],
		points[(xx + 1) % MOVE_POINTS],
		points[(xx + 2) % MOVE_POINTS], s + 0.001) - glm::catmullRom(
			points[(xx - 1) % MOVE_POINTS],
			points[(xx) % MOVE_POINTS],
			points[(xx + 1) % MOVE_POINTS],
			points[(xx + 2) % MOVE_POINTS], s - 0.001));


	glm::vec3 position = glm::catmullRom(
		points[(xx - 1) % MOVE_POINTS],
		points[(xx) % MOVE_POINTS],
		points[(xx + 1) % MOVE_POINTS],
		points[(xx + 2) % MOVE_POINTS],
		s);

	glm::vec3 side = glm::cross(forward, up);

	glm::mat4 movementRotation;
	movementRotation[0][0] = side.x; movementRotation[1][0] = side.y; movementRotation[2][0] = side.z;
	movementRotation[0][1] = up.x; movementRotation[1][1] = up.y; movementRotation[2][1] = up.z;
	movementRotation[0][2] = -forward.x; movementRotation[1][2] = -forward.y; movementRotation[2][2] = -forward.z;

	glm::mat4 movementTranslation;
	movementTranslation[3] = glm::vec4(-position, 1.0f);

	return movementRotation * movementTranslation;
}

glm::mat4 createCameraMatrix(int cameraVelocityINT)
{
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - appLoadingTime;
	glm::vec3 up = glm::vec3(0, 1, 0);

	if (isManual == true) {
		cameraDir = glm::vec3(cosf(cameraAngle), 0.0f, sinf(cameraAngle));
	}

	if (isManual == false) {
		float s = glm::fract(time / cameraVelocityINT);
		int xx = floorf(time / cameraVelocityINT);

		cameraDir = glm::normalize(glm::catmullRom(
			cameraKeyPoints[(xx - 1) % NUM_CAMERA_POINTS],
			cameraKeyPoints[(xx) % NUM_CAMERA_POINTS],
			cameraKeyPoints[(xx + 1) % NUM_CAMERA_POINTS],
			cameraKeyPoints[(xx + 2) % NUM_CAMERA_POINTS], s + 0.001) - glm::catmullRom(
				cameraKeyPoints[(xx - 1) % NUM_CAMERA_POINTS],
				cameraKeyPoints[(xx) % NUM_CAMERA_POINTS],
				cameraKeyPoints[(xx + 1) % NUM_CAMERA_POINTS],
				cameraKeyPoints[(xx + 2) % NUM_CAMERA_POINTS], s - 0.001));

		cameraPos = glm::catmullRom(
			cameraKeyPoints[(xx - 1) % NUM_CAMERA_POINTS],
			cameraKeyPoints[(xx) % NUM_CAMERA_POINTS],
			cameraKeyPoints[(xx + 1) % NUM_CAMERA_POINTS],
			cameraKeyPoints[(xx + 2) % NUM_CAMERA_POINTS],
			s);
	}

	return Core::createViewMatrix(cameraPos, cameraDir, up);
}

void drawObjectSharpStripes(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 colorA, glm::vec3 colorB, float stripeWidth)
{
	GLuint program = programSharpStripes;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColorA"), colorA.x, colorA.y, colorA.z);
	glUniform3f(glGetUniformLocation(program, "objectColorB"), colorB.x, colorB.y, colorB.z);
	glUniform1f(glGetUniformLocation(program, "stripeWidth"), stripeWidth);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectFadedStripes(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 colorA, glm::vec3 colorB, float stripeWidth)
{
	GLuint program = programFadedStripes;

	glUseProgram(program);
	glUniform3f(glGetUniformLocation(program, "objectColorA"), colorA.x, colorA.y, colorA.z);
	glUniform3f(glGetUniformLocation(program, "objectColorB"), colorB.x, colorB.y, colorB.z);
	glUniform1f(glGetUniformLocation(program, "stripeWidth"), stripeWidth);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectPerlin(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programPerlin;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}
void drawObjectVoro(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - appLoadingTime_forShaders;

	GLuint program = programVoro;

	glUseProgram(program);
	glUniform1f(glGetUniformLocation(program, "time"), time);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}
void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTextureSpecular(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTextureSpecular;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void renderScene()
{
	cameraMatrix = createCameraMatrix(cameraVelocityINT);
	perspectiveMatrix = Core::createPerspectiveMatrix(.1, 140);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.49, 0.69, 0.76, 1.0f);

	cameraAngle = atan2f(cameraDir.z, cameraDir.x); // obrot kamery w celu widoku z boku karasia

													//// 0 / 1000, -0.25,0 kara pospolity                                                                    //// 
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(1000, -0.25f, 0)) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.2f));

	drawObjectTextureSpecular(&fishModel, shipModelMatrix, texFish);
	drawObjectFadedStripes(&sand, glm::translate(glm::vec3(49, -19, -30)) * glm::scale(glm::vec3(10.2f)), glm::vec3(0.91, 0.91, 0.85), glm::vec3(0.91, 0.85, 0.56), 2);
	drawObjectFadedStripes(&ruins, glm::translate(glm::vec3(-50, -16, -65)) * glm::scale(glm::vec3(.15f)), glm::vec3(0.79, 0.79, 0.77), glm::vec3(0.93, 0.93, 0.91), 40);
	drawObjectColor(&reef, glm::translate(glm::vec3(3, -14.5, -10)) * glm::scale(glm::vec3(.8f)), glm::vec3(0.44, 0.56, 0.27));

	drawObjectColor(&plant, glm::translate(glm::vec3(5, -15, -4)) * glm::scale(glm::vec3(1.9f)), glm::vec3(0.43, 0.83, 0.51));
	drawObjectColor(&plant, glm::translate(glm::vec3(-6, -14.7, -10)) * glm::scale(glm::vec3(2.1f)), glm::vec3(0.04, 0.93, 0.29));
	drawObjectColor(&plant, glm::translate(glm::vec3(12, -15, -19)) * glm::scale(glm::vec3(2.6f)), glm::vec3(0.04, 0.93, 0.29));
	drawObjectColor(&plant, glm::translate(glm::vec3(-9, -15.8, -28)) * glm::scale(glm::vec3(1.8f)), glm::vec3(0.43, 0.83, 0.51));
	drawObjectColor(&plant, glm::translate(glm::vec3(9, -14., -28)) * glm::scale(glm::vec3(1.8f)), glm::vec3(0.04, 0.93, 0.29));


	int bbb = 1;
	for (int i = 0; i < NUM_FISHSPOTS; i++)
	{
		if (i == NUM_FISHSPOTS - 1) {
			drawObjectColor(&sharkModel, glm::translate(fishStartPositions[i] - glm::vec3(5.9, -2, 5)) * createFishMovementMatrix(sharkMovementPoints, 1.5) * glm::scale(glm::vec3(.6f)), glm::vec3(0.41, 0.41, 0.4));
			break;
		}

		if (bbb % 6 == 0) {
			for (float j = 2.0; j < 6; j += 0.6) {
				drawObjectFadedStripes(&f2, glm::translate(fishStartPositions[i] - glm::vec3(6.1, j, 0))*createFishMovementMatrix(fishMovementPoints, j) * glm::scale(glm::vec3(0.13f)), glm::vec3(0.32, 0.75, 0.12), glm::vec3(0.85, j / 5, 0.12), 1.0);
			}
		}

		if (bbb % 6 == 1) {
			for (float j = 2.0; j < 6; j += 0.3) { //do popr
				drawObjectSharpStripes(&f5, glm::translate(fishStartPositions[i] - glm::vec3(j, j*1.5, j += 0.2))*createFishMovementMatrix(fishMovementPoints, j*.7) * glm::scale(glm::vec3(0.13f)), glm::vec3(0.32, 0.75, 0.12), glm::vec3(0.75, 0.24, 0.12), .5);
			}
			for (float j = 1.5; j < 4; j += 0.6) { //VORONOI / Worley noise / Fractional Brownian motion 
				drawObjectVoro(&f4, glm::translate(fishStartPositions[i] - glm::vec3(2.1, j, 0))*createFishMovementMatrix(fishMovementPoints, .8*j) * glm::scale(glm::vec3(0.15f)), glm::vec3(j / 5, 0.5, 0.2));
			}
			for (float j = 1.5, k = 8; j < 5; j += 0.6, k -= .8) {
				drawObjectPerlin(&f3, glm::translate(fishStartPositions[i] - glm::vec3(5.1, j, 0))*createFishMovementMatrix(fishMovementPoints, .9*j) * glm::scale(glm::vec3(0.1f)), glm::vec3(j / 5.2, 0.3, k / 8));
			}
		}

		if (bbb % 6 == 2) {
			drawObjectFadedStripes(&f2, glm::translate(fishStartPositions[i])*createFishMovementMatrix(fishMovementPoints, 2) * glm::scale(glm::vec3(0.15f)), glm::vec3(0.95, 0.97, 0.26), glm::vec3(0.91, 0.54, 0.14), .9);
			drawObjectTextureSpecular(&fishModel, glm::translate(fishStartPositions[i])*createFishMovementMatrix(fishMovementPoints, 3)* glm::scale(glm::vec3(.25f)), texFish);
		}

		if (bbb % 6 == 3) {
			for (float j = 2.5, k = 6.0; j < 4; j += 0.4, k -= 0.5) {
				drawObjectFadedStripes(&f5, glm::translate(fishStartPositions[i] - glm::vec3(3.1, j, k / 3))*createFishMovementMatrix(fishMovementPoints, 0.7*j) * glm::scale(glm::vec3(0.1f)), glm::vec3(0.93, 0.6, 0.05), glm::vec3(j / 8, 0.4, k / 8), 1.1);
				drawObjectSharpStripes(&f5, glm::translate(fishStartPositions[i] - glm::vec3(j, j*1.5, j += 0.1))*createFishMovementMatrix(fishMovementPoints, j*.7) * glm::scale(glm::vec3(0.1f)), glm::vec3(0.32, 0.75, 0.12), glm::vec3(0.75, 0.24, 0.12), .5);
			}
		}

		if (bbb % 6 == 4) {
			drawObjectPerlin(&fishModel, glm::translate(fishStartPositions[i] - glm::vec3(.0, .0, 0))*createFishMovementMatrix(fishMovementPoints, 2) * glm::scale(glm::vec3(0.15f)), glm::vec3(0.76, 0.35, 0.18));
			drawObjectPerlin(&fishModel, glm::translate(fishStartPositions[i] - glm::vec3(.1, 2.2, 1))*createFishMovementMatrix(fishMovementPoints, 2) * glm::scale(glm::vec3(0.1f)), glm::vec3(0.71, 0.68, 0.83));
		}

		if (bbb % 6 == 5) {
			for (float j = 1.5, k = 6.0; j < 4; j += 0.6, k -= 0.3) {
				drawObjectPerlin(&f4, glm::translate(fishStartPositions[i] - glm::vec3(2.1, j, k / 7))*createFishMovementMatrix(fishMovementPoints, 1.2*j) * glm::scale(glm::vec3(0.1f)), glm::vec3(j / 5, 0.4, k / 8));
			}
		}
		bbb++;
	}


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < NUM_BUBBLES; i++)
	{
		drawObjectTexture(&bubble, glm::translate(bubbleStartPoints[i] - glm::vec3(0, 0, 0))*createBubbleMovementMatrixAlpha() * glm::scale(glm::vec3(bubbleSizes[i])), texBubble);
	}

	for (int i = 0; i < NUM_BUBBLES; i++)
	{
		drawObjectTexture(&bubble, glm::translate(bubbleStartPoints[i] - glm::vec3(0, 30, 0))*createBubbleMovementMatrixBeta() * glm::scale(glm::vec3(.04f)), texBubble);
	}
	glDisable(GL_BLEND);


	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programPerlin = shaderLoader.CreateProgram("shaders/shader_proc.vert", "shaders/shader_perlin.frag");
	programFadedStripes = shaderLoader.CreateProgram("shaders/shader_proc.vert", "shaders/shader_faded_stripes.frag");
	programSharpStripes = shaderLoader.CreateProgram("shaders/shader_proc.vert", "shaders/shader_sharp_stripes.frag");
	programVoro = shaderLoader.CreateProgram("shaders/shader_proc.vert", "shaders/shader_voro_noise.frag");
	programTextureSpecular = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex_specular.frag");

	sand = obj::loadModelFromFile("models/sand.obj");
	fishModel = obj::loadModelFromFile("models/fish.obj");
	f2 = obj::loadModelFromFile("models/f2.obj");
	f3 = obj::loadModelFromFile("models/f3.obj");
	f4 = obj::loadModelFromFile("models/f4.obj");
	f5 = obj::loadModelFromFile("models/f5.obj");
	ruins = obj::loadModelFromFile("models/ruins.obj");
	sharkModel = obj::loadModelFromFile("models/shark.obj");
	bubble = obj::loadModelFromFile("models/bubble.obj");
	plant = obj::loadModelFromFile("models/plant.obj");
	reef = obj::loadModelFromFile("models/reef.obj");

	texBubble = Core::LoadTexture("textures/bubble.png");
	texFish = Core::LoadTexture("textures/fish.png");

	static const float astRadius = 6.0;
	for (int i = 0; i < NUM_FISHSPOTS; i++)
	{
		float angle = (float(i))*(2 * glm::pi<float>() / NUM_FISHSPOTS);
		fishStartPositions[i] = glm::vec3(cosf(angle), 0.0f, sinf(angle)) * astRadius + glm::sphericalRand(0.5f);
		//fishStartPositions[i] = glm::ballRand(6.0);
	}

	for (int i = 0; i < NUM_BUBBLES; i++)
	{
		bubbleStartPoints[i] = glm::ballRand(16.0);
		bubbleSizes[i] = glm::linearRand(0.03f, 0.06f);
		//bubbleStartPoints[i] -= glm::vec3(0, 10, 0);
	}

	static const float camRadius = 3.55;
	static const float camOffset = 0.6;
	for (int i = 0; i < NUM_CAMERA_POINTS; i++)
	{
		float angle = (float(i))*(2 * glm::pi<float>() / NUM_CAMERA_POINTS);
		float radius = camRadius *(0.95 + glm::linearRand(0.0f, 0.15f));
		cameraKeyPoints[i] = glm::vec3(cosf(angle) + camOffset, 0.0f, sinf(angle)) * radius;
	}


	for (int i = 0; i < MOVE_POINTS; i++)
	{
		float angle = (float(i))*(2 * glm::pi<float>() / MOVE_POINTS);
		float radius = 4.5 *(0.95 + glm::linearRand(0.00f, 0.0003f));
		fishMovementPoints[i] = glm::vec3(cosf(angle) + 1.7, 0.0f, sinf(angle)) * radius;
	}											  ////


	for (int i = 0; i < MOVE_POINTS; i++)
	{
		float angle = (float(i))*(2 * glm::pi<float>() / MOVE_POINTS);
		float radius = 7.5 *(0.95 + glm::linearRand(0.0f, 0.01f));
		sharkMovementPoints[i] = glm::vec3(cosf(angle) + 1.6, 0.0f, sinf(angle)) * radius;
	}										  ////

	appLoadingTime_forShaders = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	appLoadingTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(programFadedStripes);
	shaderLoader.DeleteProgram(programPerlin);
	shaderLoader.DeleteProgram(programTextureSpecular);
	shaderLoader.DeleteProgram(programVoro);
	shaderLoader.DeleteProgram(programSharpStripes);

}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(1920, 1080);
	glutCreateWindow("OpenGL Projekt GRK");
	glutFullScreen();

	glewInit();
	init();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
