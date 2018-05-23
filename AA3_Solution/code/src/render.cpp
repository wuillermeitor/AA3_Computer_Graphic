#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <cassert>
#include <SDL.h>
#include <iostream>
#include <glm/gtx/compatibility.hpp>

#include "GL_framework.h"
#include <vector>

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>


//variables to load an object:

#define PI  3.141592658
#define nExercises 7
const glm::vec4 DARKBLUE = glm::vec4(100 / 255.f, 120 / 255.f, 200 / 255.f, 0);

namespace sun {
	glm::vec3 pos(0);
	glm::vec4 color(1);
	float period=20.f;
	glm::vec4 ambient=glm::vec4(1);
}

namespace moon {
	glm::vec3 pos(0);
	glm::vec4 color(1);
	float period=9.f;
}

namespace bulb {
	glm::vec3 pos(0);
	glm::vec4 color(1);
}

glm::vec3 sunPos;
glm::vec3 moonPos;
glm::vec3 bulbPos;

extern bool loadOBJ(const char * path,
	std::vector < glm::vec3 > & out_vertices,
	std::vector < glm::vec2 > & out_uvs,
	std::vector < glm::vec3 > & out_normals
);

namespace globalVariables {
	int exCounter = 1;
	int cameraCounter = 0;
	bool models = true;
	bool pressed = false;
	int bulbState = 0;
	int toonShading = 0;
	int contourShading = 0;
	glm::vec4 modelColor;
	float lastTime=0;
	float dt=0;
	bool trumpNoria1 = true;
}
namespace GV = globalVariables;

namespace ContourVariables {
	int countour = 1;
}
namespace CV = ContourVariables;

namespace shaders {
	int nCabinas=20;
	float rCabinas=30.f;
}


std::vector< glm::vec3 > vertices;
std::vector< glm::vec2 > uvs;
std::vector< glm::vec3 > normals;

namespace trump {
	GLuint modelVao;
	GLuint modelVbo[3];
	glm::vec4 color={1, 0, 0, 0};

	glm::mat4 objMat = glm::mat4(1.f);
}

namespace chicken {
	GLuint modelVao;
	GLuint modelVbo[3];
	glm::vec4 color = { 0, 1, 0, 0 };

	glm::mat4 objMat = glm::mat4(1.f);
}

namespace cabina {
	GLuint* modelVao=new GLuint[shaders::nCabinas];
	GLuint* modelVbo=new GLuint[shaders::nCabinas *3];

	glm::mat4* objMat = new glm::mat4[shaders::nCabinas];
}

namespace pata {
	GLuint modelVao;
	GLuint modelVbo[3];

	glm::mat4 objMat = glm::mat4(1.f);
}

namespace noria {
	GLuint modelVao;
	GLuint modelVbo[3];
	glm::vec4 color = { 196/255.f, 144 / 255.f, 204 / 255.f, 0 };

	glm::mat4 objMat = glm::mat4(1.f);
}

namespace contourTrump {
	GLuint modelVao;
	GLuint modelVbo[3];
	glm::vec4 color = { 0, 0, 0, 0 };


	glm::mat4 objMat = glm::mat4(1.f);
}

namespace contourChicken {
	GLuint modelVao;
	GLuint modelVbo[3];
	glm::vec4 color = { 0, 1, 0, 0 };

	glm::mat4 objMat = glm::mat4(1.f);
}

namespace contourCabina {
	GLuint* modelVao = new GLuint[shaders::nCabinas];
	GLuint* modelVbo = new GLuint[shaders::nCabinas * 3];

	glm::mat4* objMat = new glm::mat4[shaders::nCabinas];
}

namespace contourPata {
	GLuint modelVao;
	GLuint modelVbo[3];

	glm::mat4 objMat = glm::mat4(1.f);
}

namespace contourNoria {
	GLuint modelVao;
	GLuint modelVbo[3];
	glm::vec4 color = { 196 / 255.f, 144 / 255.f, 204 / 255.f, 0 };

	glm::mat4 objMat = glm::mat4(1.f);
}

bool show_test_window = false;

float temp1 = 0;
float temp2 = 0;
float temp3 = 0;

bool light_moves = true;

void GUI() {
	bool show = true;
	ImGui::Begin("Simulation Parameters", &show, 0);

	// Do your GUI code here....
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate

		ImGui::Text("Current Exercise: %d", GV::exCounter);

		switch (GV::exCounter) {
		case 1: //solo pinta noria de cubos
			ImGui::Text("\nDescription: ");
			ImGui::Text("\tA set of cubes whose movement resembles the one of a wheel (noria).");
			break;
		case 2: //noria de modelos
			ImGui::Text("\nDescription: ");
			ImGui::Text("\tSame behaviour that the one in the first exercise. \n\tThis time using and rendering 3d meshes to compose an actual wheel.");
			break;
		case 3: //noria de modelos + movimientos de cámara
			switch (GV::cameraCounter) {
			case 0:
				ImGui::Text("Current Camera: General Shot");
				break;
			case 1:
				ImGui::Text("Current Camera: Shot Counter Shot");
				break;
			case 2:
				ImGui::Text("Current Camera: Lateral View");
				break;
			case 3:
				ImGui::Text("Current Camera: Rotating God's Eye Shot");
				break;
			}
			ImGui::Text("\nDescription: ");
			ImGui::Text("\tIn this exercise you can change the camera position by pressing the C key. \n\tEverything else remains the same as in the last exercise.");
			break;
		case 4: //noria de modelos + fuentes de luz
			if (light_moves)
				ImGui::Text("D key to Day-Night Transition is: On");
			else {
				ImGui::Text("D key to Day-Night Transition is: Off");

				if (GV::bulbState == 0)
					ImGui::Text("B key to Bulb Light: On (Static)");
				else if (GV::bulbState == 1)
					ImGui::Text("B key to Bulb Light: Off");
				else if (GV::bulbState == 2)
					ImGui::Text("B key to Bulb Light: On (Pendulum)");
			}
			ImGui::Text("\nDescription: ");
			ImGui::Text("\tThis time the wheel is lightened by different light sources. \n\tWe have the Sun and the Moon, whose movement may be stopped by pressing the D key.\n\
	In this mode the Sun will disappear and the Moon and the dark blue ambient light will be the only ones \n\tlightening the scene.\n\
	Whenever the night-day cycle is turned off, you can press B to change between the different bulb light illumination modes. \n\ \n\
	The bulb light consists in a diffuse green light located between trump and the chicken. \n\
	Its modes are:\n\
		1.-Static (stays between Trump and Chicken and emits light)\n\
		2.-Off (doesn't emit light)\n\
		3.-Pendulum (the bulb oscilates like a pendulum in the plane between Trump and the Chicken, while emmiting light) ");
			break;
		case 5: //global scene composition
			switch (GV::cameraCounter) {
			case 0:
				ImGui::Text("Current Camera: General Shot");
				break;
			case 1:
				ImGui::Text("Current Camera: Shot Counter Shot");
				break;
			case 2:
				ImGui::Text("Current Camera: Lateral View");
				break;
			case 3:
				ImGui::Text("Current Camera: Rotating God's Eye Shot");
				break;
			}
			ImGui::Text("\nDescription: ");
			ImGui::Text("\tIn This scene there are two wheel and you can change between camera modes, \n\
	the same way you could in exercise 3.\n\
	Whenever the cabin where Trump and the Chicken reaches its wheel's lowest point,\n\
	both charachters teleport to the other wheel's topmost cabin.");
			break;
		case 6: //toon shading exercises
			if (light_moves)
				ImGui::Text("D key to Day-Night Transition is: On");
			else {
				ImGui::Text("D key to Day-Night Transition is: Off");

				if (GV::bulbState == 0)
					ImGui::Text("B key to Bulb Light: On (Static)");
				else if (GV::bulbState == 1)
					ImGui::Text("B key to Bulb Light: Off");
				else if (GV::bulbState == 2)
					ImGui::Text("B key to Bulb Light: On (Pendulum)");
			}
			if (GV::toonShading == 0) {
				ImGui::Text("T key to Toon Shading: Exercise 9 (1 - Only Sun)");
			}
			else if (GV::toonShading == 1) {
				ImGui::Text("T key to Toon Shading: Exercise 10 (2 - Sun and Moon)");
			}
			else if (GV::toonShading == 2) {
				ImGui::Text("T key to Toon Shading: Exercise 11 (3 - Night Illumination and Bulb Light)");
			}
			else if (GV::toonShading == 3) {
				ImGui::Text("T key to Toon Shading: Exercise 9 , 10 and 11 (4 - Toon Shading Off)");
			}
			ImGui::Text("\nDescription: ");
			ImGui::Text("\tKeeping the day-night and bulb options you had in exercise 4,\n\
	in this exercise you can also press T to change between the different shading modes:\n\
\n\		1.-Toon Shading with only sunlight\n\
		2.-Toon Shading with both Sun and Moon light\n\
		3.-Toon Shading with night and bulb illumination\n\
		4.-Toon Shading Off");
			break;
		case 7: //contour shading
			if (GV::contourShading == 0) {
				ImGui::Text("T key to Contour Shading: Exercise 12 (1 - Trump Contour)");
			}
			else if (GV::contourShading == 1) {
				ImGui::Text("T key to Contour Shading: Exercise 13 (2 - Trump Highlight Contour)");
			}
			else if (GV::contourShading == 2) {
				ImGui::Text("T key to Contour Shading: Exercise 14 (3 - Thick characters Contour)");
			}
			else
				std::cout << "algo va mal" << std::endl;
			switch (GV::cameraCounter) {
			case 0:
				ImGui::Text("Current Camera: General Shot");
				break;
			case 1:
				ImGui::Text("Current Camera: Shot Counter Shot");
				break;
			case 2:
				ImGui::Text("Current Camera: Lateral View");
				break;
			case 3:
				ImGui::Text("Current Camera: Rotating God's Eye Shot");
				break;
			}
			ImGui::Text("\nDescription: ");
			ImGui::Text("\tKeeping the camera modes you had on exercise 3, in this exercise you can\n\
	change between different outlining modes. The modes are:\n\
		1.-Trump Black Outline\n\
		2.-Trump Yellow Outline\n\
		3.-Characters thick otline and wheel thin outline.\n\ ");
			break;
		}
	}
	// .........................

	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 60), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

///////// fw decl
namespace ImGui {
	void Render();
}
namespace Box {
void setupCube();
void cleanupCube();
void drawCube();
}
namespace Axis {
void setupAxis();
void cleanupAxis();
void drawAxis();
}

namespace MyLoadedModel {
	void setupModel(int model);
	void cleanupModel(int model);
	void updateModel(const glm::mat4& transform, int model, int cabina=0);
	void drawModel(int model, int cabina=0);
}

namespace Sphere {
	void setupSphere(glm::vec3 pos, float radius);
	void cleanupSphere();
	void updateSphere(glm::vec3 pos, float radius);
	void drawSphere();
}

namespace Cube {
	void setupCube();
	void cleanupCube();
	void updateCube(const glm::mat4& transform);
	void drawCube(float currentTime);
}

void noriaRelocateEx5(float currentTime, float offset);


////////////////

namespace RenderVars {
	const float FOV = glm::radians(65.f);
	const float zNear = 1.f;
	const float zFar = 500.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -0.f, -0.f };
	float rota[2] = { 0.f, 0.f };
}
namespace RV = RenderVars;

void GLResize(int width, int height) {
	glViewport(0, 0, width, height);
	if(height != 0) RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	else RV::_projection = glm::perspective(RV::FOV, 0.f, RV::zNear, RV::zFar);
}

void GLmousecb(MouseEvent ev) {
	if(RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) {
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch(ev.button) {
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	} else {
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}

void GLinit(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(RV::FOV, (float)width/(float)height, RV::zNear, RV::zFar);

	// Setup shaders & geometry
	/*Box::setupCube();
	Axis::setupAxis();*/

	for (int i = 0; i < shaders::nCabinas; ++i) {
		cabina::objMat[i] = glm::mat4(1.f);
	}

	bool res = loadOBJ("trump.obj", vertices, uvs, normals);
	for (int i = 0; i < vertices.size(); ++i) {
			vertices.at(i) /= 80;
	}
	MyLoadedModel::setupModel(0);
	MyLoadedModel::setupModel(5);

	vertices.clear();
	vertices.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();
	uvs.clear();
	uvs.shrink_to_fit();

	res = loadOBJ("chicken.obj", vertices, uvs, normals);
	for (int i = 0; i < vertices.size(); ++i) {
		vertices.at(i) /= 60;
	}
	MyLoadedModel::setupModel(1);
	MyLoadedModel::setupModel(6);

	vertices.clear();
	vertices.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();
	uvs.clear();
	uvs.shrink_to_fit();

	res = loadOBJ("NoriaCabina.obj", vertices, uvs, normals);
	for (int i = 0; i < vertices.size(); ++i) {
		vertices.at(i) /= 180;
	}
	MyLoadedModel::setupModel(2);
	MyLoadedModel::setupModel(7);

	vertices.clear();
	vertices.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();
	uvs.clear();
	uvs.shrink_to_fit();

	res = loadOBJ("NoriaFeet.obj", vertices, uvs, normals);
	for (int i = 0; i < vertices.size(); ++i) {
		vertices.at(i) /= 92.5f;
	}
	MyLoadedModel::setupModel(3);
	MyLoadedModel::setupModel(8);

	vertices.clear();
	vertices.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();
	uvs.clear();
	uvs.shrink_to_fit();

	res = loadOBJ("NoriaRueda.obj", vertices, uvs, normals);
	for (int i = 0; i < vertices.size(); ++i) {
		vertices.at(i) /= 92.5f;
	}
	MyLoadedModel::setupModel(4);
	MyLoadedModel::setupModel(9);

	vertices.clear();
	vertices.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();
	uvs.clear();
	uvs.shrink_to_fit();


	Sphere::setupSphere(moonPos, 1.0f);
	Cube::setupCube();
	moonPos = glm::vec3(40, 40, 0);

}

void GLcleanup() {
	/*Box::cleanupCube();
	Axis::cleanupAxis();*/
	for (int i = 0; i < 10; ++i)
	{
		MyLoadedModel::cleanupModel(i);
	}
	Sphere::cleanupSphere();
	Cube::cleanupCube();


}

static double timeGiratorio = 0;
void GLrender(double currentTime) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	Cube::drawCube(currentTime);

	switch (GV::exCounter) {
	case 1: //solo pinta noria de cubos
		break;
	case 2: //noria de modelos

		sun::color = sun::ambient = moon::color = bulb::color = { 1, 1, 1, 0 };

		MyLoadedModel::drawModel(0);
		MyLoadedModel::drawModel(1);
		MyLoadedModel::drawModel(3);
		MyLoadedModel::drawModel(4);
		for (int i = 0; i < shaders::nCabinas; ++i)
			MyLoadedModel::drawModel(2, i);
		break;
	case 3: //noria de modelos + movimientos de cámara

		sun::color = sun::ambient = moon::color = bulb::color = { 1, 1, 1, 0 };

		MyLoadedModel::drawModel(0);
		MyLoadedModel::drawModel(1);
		MyLoadedModel::drawModel(3);
		MyLoadedModel::drawModel(4);
		for (int i = 0; i < shaders::nCabinas; ++i)
			MyLoadedModel::drawModel(2, i);
		break;
	case 4: { //noria de modelos + fuentes de luz
			//time giratorio
		if (light_moves) {
			timeGiratorio += GV::dt;
		}
		//ACTUALIZAR POSICIÓN Y COLOR DE LOS ASTROS
		float circleRad = 40;
		float ySinSol = sin(glm::radians((float)(timeGiratorio / sun::period) * 360));
		float zCosSol = cos(glm::radians((float)(timeGiratorio / sun::period) * 360));
		float ySinLuna = sin(glm::radians((float)(timeGiratorio / moon::period) * 360));
		float zCosLuna = cos(glm::radians((float)(timeGiratorio / moon::period) * 360));

		if (light_moves) {
			//POSICIONES EN FUNCIÓN DEL SENO Y COSENO DE SOL Y LUNA
			moon::pos = glm::rotate(glm::mat4(1), glm::radians(135.f), glm::vec3(0, 1, 0)) * glm::vec4(0, ySinLuna*circleRad, zCosLuna*circleRad, 1);
			sun::pos = glm::vec3(0, ySinSol*circleRad, zCosSol*circleRad);

			//hacemos que los senos de estos valores vayan de 0 a 2
			ySinSol += 1;
			zCosSol += 1;
			ySinLuna += 1;

			//COLORES EN FUNCIÓN DE LA POSICIÓN
			//luna
			moon::color = glm::vec4(glm::lerp(glm::vec3(0, 0, 0), glm::vec3(172.f / 255.f, 220.f / 255.f, 221.f / 255.f), ySinLuna), 0);

			//sol
			if (ySinSol < 1) {
				sun::color = glm::vec4(glm::lerp(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), ySinSol), 0);
				sun::ambient = DARKBLUE;
			}
			else {
				sun::color = glm::vec4(glm::lerp(glm::vec3(1, 0, 0), glm::vec3(1, 1, 0.8), ySinSol - 1.f), 0);
				sun::ambient = glm::vec4(1);
			}

			//bulb
			bulb::color = { 0, 0, 0, 0 };
		}
		else {
			//ACTUALIZACIÓN DE POSICIONES

			//sol nada
			//luna nada
			//bulb
			if (GV::bulbState == 2) { //if pendulo
				bulb::pos.y += -glm::abs(cos(currentTime)) * 5.f;
				bulb::pos.z += sin(currentTime) * 10;
			}

			//COLORES

			//luna
			moon::color = glm::vec4(glm::lerp(glm::vec3(0, 0, 0), glm::vec3(172.f / 255.f, 220.f / 255.f, 221.f / 255.f), ySinLuna), 0);
			//sol
			sun::color = glm::vec4(0);
			//bulb
			if (GV::bulbState == 1) {
				bulb::color = glm::vec4(0.f);
			}
			else {
				bulb::color = { 0, 1, 0, 0 };
			}
			//ambient
			sun::ambient = DARKBLUE;
		}

		//DRAW
		if (light_moves) { //sol
			Sphere::updateSphere(sun::pos, 1.f);
			Sphere::drawSphere();
		}
		else if (GV::bulbState == 0 || GV::bulbState == 2) { //bulb
			Sphere::updateSphere(bulb::pos, .5f);
			Sphere::drawSphere();
		}
		//luna
		Sphere::updateSphere(moon::pos, 1.f);
		Sphere::drawSphere();


		//modelos
		MyLoadedModel::drawModel(0);
		MyLoadedModel::drawModel(1);
		MyLoadedModel::drawModel(3);
		MyLoadedModel::drawModel(4);
		for (int i = 0; i < shaders::nCabinas; ++i)
			MyLoadedModel::drawModel(2, i);
	}
			break;
	case 5: //global scene composition



		sun::color = sun::ambient = moon::color = bulb::color = { 1, 1, 1, 0 };

		MyLoadedModel::drawModel(0);
		MyLoadedModel::drawModel(1);
		MyLoadedModel::drawModel(4);
		MyLoadedModel::drawModel(3);
		
		for (int i = 0; i < shaders::nCabinas; ++i)
			MyLoadedModel::drawModel(2, i);

		noriaRelocateEx5(currentTime, 65);

		MyLoadedModel::drawModel(3);
		MyLoadedModel::drawModel(4);
		for (int i = 0; i < shaders::nCabinas; ++i)
			MyLoadedModel::drawModel(2, i);

		break;
	case 6: {//toon shading exercises
			//time giratorio
		if (light_moves) {
			timeGiratorio += GV::dt;
		}
		//ACTUALIZAR POSICIÓN Y COLOR DE LOS ASTROS
		float circleRad = 40;
		float ySinSol = sin(glm::radians((float)(timeGiratorio / sun::period) * 360));
		float zCosSol = cos(glm::radians((float)(timeGiratorio / sun::period) * 360));
		float ySinLuna = sin(glm::radians((float)(timeGiratorio / moon::period) * 360));
		float zCosLuna = cos(glm::radians((float)(timeGiratorio / moon::period) * 360));

		if (light_moves) {
			//POSICIONES EN FUNCIÓN DEL SENO Y COSENO DE SOL Y LUNA
			moon::pos = glm::rotate(glm::mat4(1), glm::radians(135.f), glm::vec3(0, 1, 0)) * glm::vec4(0, ySinLuna*circleRad, zCosLuna*circleRad, 1);
			sun::pos = glm::vec3(0, ySinSol*circleRad, zCosSol*circleRad);

			//hacemos que los senos de estos valores vayan de 0 a 2
			ySinSol += 1;
			zCosSol += 1;
			ySinLuna += 1;

			//COLORES EN FUNCIÓN DE LA POSICIÓN
			//luna
			moon::color = glm::vec4(glm::lerp(glm::vec3(0, 0, 0), glm::vec3(172.f / 255.f, 220.f / 255.f, 221.f / 255.f), ySinLuna), 0);

			//sol
			if (GV::toonShading == 2) {
				bulb::pos.y += -glm::abs(cos(currentTime)) * 10;
				bulb::pos.x += sin(currentTime) * 20;
				bulb::color = { 0, 1, 0, 0 };
				sun::ambient = DARKBLUE;
			}
			else {
				if (ySinSol < 1) {
					sun::color = glm::vec4(glm::lerp(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), ySinSol), 0);
					sun::ambient = DARKBLUE;
				}
				else {
					sun::color = glm::vec4(glm::lerp(glm::vec3(1, 0, 0), glm::vec3(1, 1, 0.5f), ySinSol - 1.f), 0);
					sun::ambient = glm::vec4(1);
				}
			}
		}
		else {
			//ACTUALIZACIÓN DE POSICIONES

			//sol nada
			//luna nada
			//bulb
			if (GV::bulbState == 2) { //if pendulo
				bulb::pos.y += -glm::abs(cos(currentTime)) * 10;
				bulb::pos.x += sin(currentTime) * 20;
			}

			//COLORES

			//luna
			moon::color = glm::vec4(glm::lerp(glm::vec3(0, 0, 0), glm::vec3(172.f / 255.f, 220.f / 255.f, 221.f / 255.f), ySinLuna), 0);
			//sol
			sun::color = glm::vec4(0);
			//bulb
			bulb::color = { 0, 1, 0, 0 };
			//ambient
			sun::ambient = DARKBLUE;
		}

		//DRAW
		if (GV::toonShading == 0 || GV::toonShading == 1 || GV::toonShading == 3) {
			Sphere::updateSphere(sun::pos, 1.f);
			Sphere::drawSphere();
		}
		
		if (GV::toonShading == 2) {
			Sphere::updateSphere(bulb::pos, .25f);
			Sphere::drawSphere();
		}
		
		//luna
		if (GV::toonShading == 1 || GV::toonShading == 2 || GV::toonShading == 3) {
			Sphere::updateSphere(moon::pos, 1.f);
			Sphere::drawSphere();
		}


		//modelos
		MyLoadedModel::drawModel(0);
		MyLoadedModel::drawModel(1);
		MyLoadedModel::drawModel(3);
		MyLoadedModel::drawModel(4);
		for (int i = 0; i < shaders::nCabinas; ++i)
			MyLoadedModel::drawModel(2, i);
	}
			break;
	case 7:
		sun::color = sun::ambient = moon::color = bulb::color = { 1, 1, 1, 0 };

		glEnable(GL_STENCIL_TEST);
		glEnable(GL_DEPTH_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glStencilMask(0x00); // make sure we don't update the stencil buffer while drawing the floor
		if (GV::contourShading != 2) {
			MyLoadedModel::drawModel(3);
			MyLoadedModel::drawModel(4);
			for (int i = 0; i < shaders::nCabinas; ++i)
				MyLoadedModel::drawModel(2, i);
			MyLoadedModel::drawModel(1);
		}

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		MyLoadedModel::drawModel(0);
		if (GV::contourShading == 2) {
			MyLoadedModel::drawModel(3);
			MyLoadedModel::drawModel(4);
			for (int i = 0; i < shaders::nCabinas; ++i)
				MyLoadedModel::drawModel(2, i);
			MyLoadedModel::drawModel(1);
		}

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
		if (GV::contourShading == 0)
			contourTrump::color = { 0,0,0,0 };
		else if (GV::contourShading == 1)
			contourTrump::color = { 1,1,0,0 };
		else {
			contourTrump::color = { 0,1,1,0 };
			contourNoria::color = { 0,1,1,0 };
			MyLoadedModel::drawModel(8);
			MyLoadedModel::drawModel(9);
			for (int i = 0; i < shaders::nCabinas; ++i)
				MyLoadedModel::drawModel(7, i);
			contourChicken::color = { 0,1,1,0 };
			MyLoadedModel::drawModel(6);
		}
		MyLoadedModel::drawModel(5);
		glStencilMask(0xFF);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);

		break;
	}
	ImGui::Render();
}


//////////////////////////////////// COMPILE AND LINK
GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name="") {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
void linkProgram(GLuint program) {
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}

////////////////////////////////////////////////// BOX
namespace Box{
GLuint cubeVao;
GLuint cubeVbo[2];
GLuint cubeShaders[2];
GLuint cubeProgram;

float cubeVerts[] = {
	// -5,0,-5 -- 5, 10, 5
	-5.f,  0.f, -5.f,
	 5.f,  0.f, -5.f,
	 5.f,  0.f,  5.f,
	-5.f,  0.f,  5.f,
	-5.f, 10.f, -5.f,
	 5.f, 10.f, -5.f,
	 5.f, 10.f,  5.f,
	-5.f, 10.f,  5.f,
};
GLubyte cubeIdx[] = {
	1, 0, 2, 3, // Floor - TriangleStrip
	0, 1, 5, 4, // Wall - Lines
	1, 2, 6, 5, // Wall - Lines
	2, 3, 7, 6, // Wall - Lines
	3, 0, 4, 7  // Wall - Lines
};

const char* vertShader_xform =
"#version 330\n\
in vec3 in_Position;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
}";
const char* fragShader_flatColor =
"#version 330\n\
out vec4 out_Color;\n\
uniform vec4 color;\n\
void main() {\n\
	out_Color = color;\n\
}";

void setupCube() {
	glGenVertexArrays(1, &cubeVao);
	glBindVertexArray(cubeVao);
	glGenBuffers(2, cubeVbo);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, cubeVerts, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 20, cubeIdx, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	cubeShaders[0] = compileShader(vertShader_xform, GL_VERTEX_SHADER, "cubeVert");
	cubeShaders[1] = compileShader(fragShader_flatColor, GL_FRAGMENT_SHADER, "cubeFrag");

	cubeProgram = glCreateProgram();
	glAttachShader(cubeProgram, cubeShaders[0]);
	glAttachShader(cubeProgram, cubeShaders[1]);
	glBindAttribLocation(cubeProgram, 0, "in_Position");
	linkProgram(cubeProgram);
}
void cleanupCube() {
	glDeleteBuffers(2, cubeVbo);
	glDeleteVertexArrays(1, &cubeVao);

	glDeleteProgram(cubeProgram);
	glDeleteShader(cubeShaders[0]);
	glDeleteShader(cubeShaders[1]);
}
void drawCube() {
	glBindVertexArray(cubeVao);
	glUseProgram(cubeProgram);
	glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
	// FLOOR
	glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.6f, 0.6f, 0.6f, 1.f);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
	// WALLS
	glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.f, 0.f, 0.f, 1.f);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 4));
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 8));
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 12));
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, (void*)(sizeof(GLubyte) * 16));

	glUseProgram(0);
	glBindVertexArray(0);
}
}

////////////////////////////////////////////////// AXIS
namespace Axis {
GLuint AxisVao;
GLuint AxisVbo[3];
GLuint AxisShader[2];
GLuint AxisProgram;

float AxisVerts[] = {
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 0.0, 0.0,
	0.0, 0.0, 1.0
};
float AxisColors[] = {
	1.0, 0.0, 0.0, 1.0,
	1.0, 0.0, 0.0, 1.0,
	0.0, 1.0, 0.0, 1.0,
	0.0, 1.0, 0.0, 1.0,
	0.0, 0.0, 1.0, 1.0,
	0.0, 0.0, 1.0, 1.0
};
GLubyte AxisIdx[] = {
	0, 1,
	2, 3,
	4, 5
};
const char* Axis_vertShader =
"#version 330\n\
in vec3 in_Position;\n\
in vec4 in_Color;\n\
out vec4 vert_color;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	vert_color = in_Color;\n\
	gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
}";
const char* Axis_fragShader =
"#version 330\n\
in vec4 vert_color;\n\
out vec4 out_Color;\n\
void main() {\n\
	out_Color = vert_color;\n\
}";

void setupAxis() {
	glGenVertexArrays(1, &AxisVao);
	glBindVertexArray(AxisVao);
	glGenBuffers(3, AxisVbo);

	glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisVerts, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisColors, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AxisVbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, AxisIdx, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	AxisShader[0] = compileShader(Axis_vertShader, GL_VERTEX_SHADER, "AxisVert");
	AxisShader[1] = compileShader(Axis_fragShader, GL_FRAGMENT_SHADER, "AxisFrag");

	AxisProgram = glCreateProgram();
	glAttachShader(AxisProgram, AxisShader[0]);
	glAttachShader(AxisProgram, AxisShader[1]);
	glBindAttribLocation(AxisProgram, 0, "in_Position");
	glBindAttribLocation(AxisProgram, 1, "in_Color");
	linkProgram(AxisProgram);
}
void cleanupAxis() {
	glDeleteBuffers(3, AxisVbo);
	glDeleteVertexArrays(1, &AxisVao);

	glDeleteProgram(AxisProgram);
	glDeleteShader(AxisShader[0]);
	glDeleteShader(AxisShader[1]);
}
void drawAxis() {
	glBindVertexArray(AxisVao);
	glUseProgram(AxisProgram);
	glUniformMatrix4fv(glGetUniformLocation(AxisProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
	glDrawElements(GL_LINES, 6, GL_UNSIGNED_BYTE, 0);

	glUseProgram(0);
	glBindVertexArray(0);
}
}

////////////////////////////////////////////////// SPHERE
namespace Sphere {
GLuint sphereVao;
GLuint sphereVbo;
GLuint sphereShaders[3];
GLuint sphereProgram;
float radius;

const char* sphere_vertShader =
"#version 330\n\
in vec3 in_Position;\n\
uniform mat4 mv_Mat;\n\
void main() {\n\
	gl_Position = mv_Mat * vec4(in_Position, 1.0);\n\
}";
const char* sphere_geomShader =
"#version 330\n\
layout(points) in;\n\
layout(triangle_strip, max_vertices = 4) out;\n\
out vec4 eyePos;\n\
out vec4 centerEyePos;\n\
uniform mat4 projMat;\n\
uniform float radius;\n\
vec4 nu_verts[4];\n\
void main() {\n\
	vec3 n = normalize(-gl_in[0].gl_Position.xyz);\n\
	vec3 up = vec3(0.0, 1.0, 0.0);\n\
	vec3 u = normalize(cross(up, n));\n\
	vec3 v = normalize(cross(n, u));\n\
	nu_verts[0] = vec4(-radius*u - radius*v, 0.0); \n\
	nu_verts[1] = vec4( radius*u - radius*v, 0.0); \n\
	nu_verts[2] = vec4(-radius*u + radius*v, 0.0); \n\
	nu_verts[3] = vec4( radius*u + radius*v, 0.0); \n\
	centerEyePos = gl_in[0].gl_Position;\n\
	for (int i = 0; i < 4; ++i) {\n\
		eyePos = (gl_in[0].gl_Position + nu_verts[i]);\n\
		gl_Position = projMat * eyePos;\n\
		EmitVertex();\n\
	}\n\
	EndPrimitive();\n\
}";
const char* sphere_fragShader_flatColor =
"#version 330\n\
in vec4 eyePos;\n\
in vec4 centerEyePos;\n\
out vec4 out_Color;\n\
uniform mat4 projMat;\n\
uniform mat4 mv_Mat;\n\
uniform vec4 color;\n\
uniform float radius;\n\
void main() {\n\
	vec4 diff = eyePos - centerEyePos;\n\
	float distSq2C = dot(diff, diff);\n\
	if (distSq2C > (radius*radius)) discard;\n\
	float h = sqrt(radius*radius - distSq2C);\n\
	vec4 nuEyePos = vec4(eyePos.xy, eyePos.z + h, 1.0);\n\
	vec4 nuPos = projMat * nuEyePos;\n\
	gl_FragDepth = ((nuPos.z / nuPos.w) + 1) * 0.5;\n\
	vec3 normal = normalize(nuEyePos - centerEyePos).xyz;\n\
	out_Color = vec4(color.xyz * dot(normal, (mv_Mat*vec4(0.0, 1.0, 0.0, 0.0)).xyz) + color.xyz * 0.3, 1.0 );\n\
}";

bool shadersCreated = false;
void createSphereShaderAndProgram() {
	if(shadersCreated) return;

	sphereShaders[0] = compileShader(sphere_vertShader, GL_VERTEX_SHADER, "sphereVert");
	sphereShaders[1] = compileShader(sphere_geomShader, GL_GEOMETRY_SHADER, "sphereGeom");
	sphereShaders[2] = compileShader(sphere_fragShader_flatColor, GL_FRAGMENT_SHADER, "sphereFrag");

	sphereProgram = glCreateProgram();
	glAttachShader(sphereProgram, sphereShaders[0]);
	glAttachShader(sphereProgram, sphereShaders[1]);
	glAttachShader(sphereProgram, sphereShaders[2]);
	glBindAttribLocation(sphereProgram, 0, "in_Position");
	linkProgram(sphereProgram);

	shadersCreated = true;
}
void cleanupSphereShaderAndProgram() {
	if(!shadersCreated) return;
	glDeleteProgram(sphereProgram);
	glDeleteShader(sphereShaders[0]);
	glDeleteShader(sphereShaders[1]);
	glDeleteShader(sphereShaders[2]);
	shadersCreated = false;
}

void setupSphere(glm::vec3 pos, float radius) {
	Sphere::radius = radius;
	glGenVertexArrays(1, &sphereVao);
	glBindVertexArray(sphereVao);
	glGenBuffers(1, &sphereVbo);

	glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, &pos, GL_DYNAMIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	createSphereShaderAndProgram();
}
void cleanupSphere() {
	glDeleteBuffers(1, &sphereVbo);
	glDeleteVertexArrays(1, &sphereVao);

	cleanupSphereShaderAndProgram();
}
void updateSphere(glm::vec3 pos, float radius) {
	glBindBuffer(GL_ARRAY_BUFFER, sphereVbo);
	float* buff = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	buff[0] = pos.x;
	buff[1] = pos.y;
	buff[2] = pos.z;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	Sphere::radius = radius;
}
void drawSphere() {
	glBindVertexArray(sphereVao);
	glUseProgram(sphereProgram);
	glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
	glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RV::_modelView));
	glUniformMatrix4fv(glGetUniformLocation(sphereProgram, "projMat"), 1, GL_FALSE, glm::value_ptr(RV::_projection));
	glUniform4f(glGetUniformLocation(sphereProgram, "color"), 0.6f, 0.1f, 0.1f, 1.f);
	glUniform1f(glGetUniformLocation(sphereProgram, "radius"), Sphere::radius);
	glDrawArrays(GL_POINTS, 0, 1);

	glUseProgram(0);
	glBindVertexArray(0);
}
}

////////////////////////////////////////////////// CAPSULE
namespace Capsule {
GLuint capsuleVao;
GLuint capsuleVbo[2];
GLuint capsuleShader[3];
GLuint capsuleProgram;
float radius;

const char* capsule_vertShader =
"#version 330\n\
in vec3 in_Position;\n\
uniform mat4 mv_Mat;\n\
void main() {\n\
	gl_Position = mv_Mat * vec4(in_Position, 1.0);\n\
}";
const char* capsule_geomShader =
"#version 330\n\
layout(lines) in; \n\
layout(triangle_strip, max_vertices = 14) out;\n\
out vec3 eyePos;\n\
out vec3 capPoints[2];\n\
uniform mat4 projMat;\n\
uniform float radius;\n\
vec3 boxVerts[8];\n\
int boxIdx[14];\n\
void main(){\n\
	vec3 A = gl_in[0].gl_Position.xyz;\n\
	vec3 B = gl_in[1].gl_Position.xyz;\n\
	if(gl_in[1].gl_Position.x < gl_in[0].gl_Position.x) {\n\
		A = gl_in[1].gl_Position.xyz;\n\
		B = gl_in[0].gl_Position.xyz;\n\
	}\n\
	vec3 u = vec3(0.0, 1.0, 0.0);\n\
	if (abs(dot(u, normalize(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz))) - 1.0 < 1e-6) {\n\
		if(gl_in[1].gl_Position.y > gl_in[0].gl_Position.y) {\n\
			A = gl_in[1].gl_Position.xyz;\n\
			B = gl_in[0].gl_Position.xyz;\n\
		}\n\
		u = vec3(1.0, 0.0, 0.0);\n\
	}\n\
	vec3 Am = normalize(A - B); \n\
	vec3 Bp = -Am;\n\
	vec3 v = normalize(cross(Am, u)) * radius;\n\
	u = normalize(cross(v, Am)) * radius;\n\
	Am *= radius;\n\
	Bp *= radius;\n\
	boxVerts[0] = A + Am - u - v;\n\
	boxVerts[1] = A + Am + u - v;\n\
	boxVerts[2] = A + Am + u + v;\n\
	boxVerts[3] = A + Am - u + v;\n\
	boxVerts[4] = B + Bp - u - v;\n\
	boxVerts[5] = B + Bp + u - v;\n\
	boxVerts[6] = B + Bp + u + v;\n\
	boxVerts[7] = B + Bp - u + v;\n\
	boxIdx = int[](0, 3, 4, 7, 6, 3, 2, 1, 6, 5, 4, 1, 0, 3);\n\
	capPoints[0] = A;\n\
	capPoints[1] = B;\n\
	for (int i = 0; i<14; ++i) {\n\
		eyePos = boxVerts[boxIdx[i]];\n\
		gl_Position = projMat * vec4(boxVerts[boxIdx[i]], 1.0);\n\
		EmitVertex();\n\
	}\n\
	EndPrimitive();\n\
}";
const char* capsule_fragShader_flatColor =
"#version 330\n\
in vec3 eyePos;\n\
in vec3 capPoints[2];\n\
out vec4 out_Color;\n\
uniform mat4 projMat;\n\
uniform mat4 mv_Mat;\n\
uniform vec4 color;\n\
uniform float radius;\n\
const int lin_steps = 30;\n\
const int bin_steps = 5;\n\
vec3 closestPointInSegment(vec3 p, vec3 a, vec3 b) {\n\
	vec3 pa = p - a, ba = b - a;\n\
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);\n\
	return a + ba*h;\n\
}\n\
void main() {\n\
	vec3 viewDir = normalize(eyePos);\n\
	float step = radius / 5.0;\n\
	vec3 nuPB = eyePos;\n\
	int i = 0;\n\
	for(i = 0; i < lin_steps; ++i) {\n\
		nuPB = eyePos + viewDir*step*i;\n\
		vec3 C = closestPointInSegment(nuPB, capPoints[0], capPoints[1]);\n\
		float dist = length(C - nuPB) - radius;\n\
		if(dist < 0.0) break;\n\
	}\n\
	if(i==lin_steps) discard;\n\
	vec3 nuPA = nuPB - viewDir*step;\n\
	vec3 C;\n\
	for(i = 0; i < bin_steps; ++i) {\n\
		vec3 nuPC = nuPA + (nuPB - nuPA)*0.5; \n\
		C = closestPointInSegment(nuPC, capPoints[0], capPoints[1]); \n\
		float dist = length(C - nuPC) - radius; \n\
		if(dist > 0.0) nuPA = nuPC; \n\
		else nuPB = nuPC; \n\
	}\n\
	vec4 nuPos = projMat * vec4(nuPA, 1.0);\n\
	gl_FragDepth = ((nuPos.z / nuPos.w) + 1) * 0.5;\n\
	vec3 normal = normalize(nuPA - C);\n\
	out_Color = vec4(color.xyz * dot(normal, (mv_Mat*vec4(0.0, 1.0, 0.0, 0.0)).xyz) + color.xyz * 0.3, 1.0 );\n\
}";

void setupCapsule(glm::vec3 posA, glm::vec3 posB, float radius) {
	Capsule::radius = radius;
	glGenVertexArrays(1, &capsuleVao);
	glBindVertexArray(capsuleVao);
	glGenBuffers(2, capsuleVbo);

	float capsuleVerts[] = {
		posA.x, posA.y, posA.z, 
		posB.x, posB.y, posB.z
	};
	GLubyte capsuleIdx[] = {
		0, 1
	};

	glBindBuffer(GL_ARRAY_BUFFER, capsuleVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, capsuleVerts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, capsuleVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 2, capsuleIdx, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	capsuleShader[0] = compileShader(capsule_vertShader, GL_VERTEX_SHADER, "capsuleVert");
	capsuleShader[1] = compileShader(capsule_geomShader, GL_GEOMETRY_SHADER, "capsuleGeom");
	capsuleShader[2] = compileShader(capsule_fragShader_flatColor, GL_FRAGMENT_SHADER, "capsuleFrag");

	capsuleProgram = glCreateProgram();
	glAttachShader(capsuleProgram, capsuleShader[0]);
	glAttachShader(capsuleProgram, capsuleShader[1]);
	glAttachShader(capsuleProgram, capsuleShader[2]);
	glBindAttribLocation(capsuleProgram, 0, "in_Position");
	linkProgram(capsuleProgram);
}
void cleanupCapsule() {
	glDeleteBuffers(2, capsuleVbo);
	glDeleteVertexArrays(1, &capsuleVao);

	glDeleteProgram(capsuleProgram);
	glDeleteShader(capsuleShader[0]);
	glDeleteShader(capsuleShader[1]);
	glDeleteShader(capsuleShader[2]);
}
void updateCapsule(glm::vec3 posA, glm::vec3 posB, float radius) {
	float vertPos[] = {posA.x, posA.y, posA.z, posB.z, posB.y, posB.z};
	glBindBuffer(GL_ARRAY_BUFFER, capsuleVbo[0]);
	float* buff = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	buff[0] = posA.x; buff[1] = posA.y; buff[2] = posA.z;
	buff[3] = posB.x; buff[4] = posB.y; buff[5] = posB.z;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	Capsule::radius = radius;
}
void drawCapsule() {
	glBindVertexArray(capsuleVao);
	glUseProgram(capsuleProgram);
	glUniformMatrix4fv(glGetUniformLocation(capsuleProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
	glUniformMatrix4fv(glGetUniformLocation(capsuleProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RV::_modelView));
	glUniformMatrix4fv(glGetUniformLocation(capsuleProgram, "projMat"), 1, GL_FALSE, glm::value_ptr(RV::_projection));
	glUniform4fv(glGetUniformLocation(capsuleProgram, "camPoint"), 1, &RV::_cameraPoint[0]);
	glUniform4f(glGetUniformLocation(capsuleProgram, "color"), 0.1f, 0.6f, 0.1f, 1.f);
	glUniform1f(glGetUniformLocation(capsuleProgram, "radius"), Capsule::radius);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_BYTE, 0);

	glUseProgram(0);
	glBindVertexArray(0);
}
}

////////////////////////////////////////////////// PARTICLES
// Same rendering as Sphere (reusing shaders)
namespace LilSpheres {
GLuint particlesVao;
GLuint particlesVbo;
float radius;
int numparticles;
extern const int maxParticles = SHRT_MAX;

void setupParticles(int numTotalParticles, float radius) {
	assert(numTotalParticles > 0);
	assert(numTotalParticles <= SHRT_MAX);
	numparticles = numTotalParticles;
	LilSpheres::radius = radius;
	
	glGenVertexArrays(1, &particlesVao);
	glBindVertexArray(particlesVao);
	glGenBuffers(1, &particlesVbo);

	glBindBuffer(GL_ARRAY_BUFFER, particlesVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numparticles, 0, GL_DYNAMIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	Sphere::createSphereShaderAndProgram();
}
void cleanupParticles() {
	glDeleteVertexArrays(1, &particlesVao);
	glDeleteBuffers(1, &particlesVbo);

	Sphere::cleanupSphereShaderAndProgram();
}
void updateParticles(int startIdx, int count, float* array_data) {
	glBindBuffer(GL_ARRAY_BUFFER, particlesVbo);
	float* buff = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	buff = &buff[3*startIdx];
	for(int i = 0; i < 3*count; ++i) {
		buff[i] = array_data[i];
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void drawParticles(int startIdx, int count) {
	glBindVertexArray(particlesVao);
	glUseProgram(Sphere::sphereProgram);
	glUniformMatrix4fv(glGetUniformLocation(Sphere::sphereProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
	glUniformMatrix4fv(glGetUniformLocation(Sphere::sphereProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RV::_modelView));
	glUniformMatrix4fv(glGetUniformLocation(Sphere::sphereProgram, "projMat"), 1, GL_FALSE, glm::value_ptr(RV::_projection));
	glUniform4f(glGetUniformLocation(Sphere::sphereProgram, "color"), 0.1f, 0.1f, 0.6f, 1.f);
	glUniform1f(glGetUniformLocation(Sphere::sphereProgram, "radius"), LilSpheres::radius);
	glDrawArrays(GL_POINTS, startIdx, count);

	glUseProgram(0);
	glBindVertexArray(0);
}
}

////////////////////////////////////////////////// CLOTH
namespace ClothMesh {
GLuint clothVao;
GLuint clothVbo[2];
GLuint clothShaders[2];
GLuint clothProgram;
extern const int numCols = 14;
extern const int numRows = 18;
extern const int numVerts = numRows * numCols;
int numVirtualVerts;

const char* cloth_vertShader =
"#version 330\n\
in vec3 in_Position;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
}";
const char* cloth_fragShader =
"#version 330\n\
uniform vec4 color;\n\
out vec4 out_Color;\n\
void main() {\n\
	out_Color = color;\n\
}";

void setupClothMesh() {
	glGenVertexArrays(1, &clothVao);
	glBindVertexArray(clothVao);
	glGenBuffers(2, clothVbo);

	glBindBuffer(GL_ARRAY_BUFFER, clothVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numVerts, 0, GL_DYNAMIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glPrimitiveRestartIndex(UCHAR_MAX);
	constexpr int facesVertsIdx = 5 * (numCols - 1) * (numRows - 1);
	GLubyte facesIdx[facesVertsIdx] = { 0 };
	for (int i = 0; i < (numRows - 1); ++i) {
		for (int j = 0; j < (numCols - 1); ++j) {
			facesIdx[5 * (i*(numCols-1) + j) + 0] = i*numCols + j;
			facesIdx[5 * (i*(numCols-1) + j) + 1] = (i + 1)*numCols + j;
			facesIdx[5 * (i*(numCols-1) + j) + 2] = (i + 1)*numCols + (j + 1);
			facesIdx[5 * (i*(numCols-1) + j) + 3] = i*numCols + (j + 1);
			facesIdx[5 * (i*(numCols-1) + j) + 4] = UCHAR_MAX;
		}
	}
	numVirtualVerts = facesVertsIdx;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clothVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*numVirtualVerts, facesIdx, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	clothShaders[0] = compileShader(cloth_vertShader, GL_VERTEX_SHADER, "clothVert");
	clothShaders[1] = compileShader(cloth_fragShader, GL_FRAGMENT_SHADER, "clothFrag");

	clothProgram = glCreateProgram();
	glAttachShader(clothProgram, clothShaders[0]);
	glAttachShader(clothProgram, clothShaders[1]);
	glBindAttribLocation(clothProgram, 0, "in_Position");
	linkProgram(clothProgram);
}
void cleanupClothMesh() {
	glDeleteBuffers(2, clothVbo);
	glDeleteVertexArrays(1, &clothVao);

	glDeleteProgram(clothProgram);
	glDeleteShader(clothShaders[0]);
	glDeleteShader(clothShaders[1]);
}
void updateClothMesh(float *array_data) {
	glBindBuffer(GL_ARRAY_BUFFER, clothVbo[0]);
	float* buff = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	for (int i = 0; i < 3 * numVerts; ++i) {
		buff[i] = array_data[i];
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void drawClothMesh() {
	glEnable(GL_PRIMITIVE_RESTART);
	glBindVertexArray(clothVao);
	glUseProgram(clothProgram);
	glUniformMatrix4fv(glGetUniformLocation(clothProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
	glUniform4f(glGetUniformLocation(clothProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
	glDrawElements(GL_LINE_LOOP, numVirtualVerts, GL_UNSIGNED_BYTE, 0);

	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_PRIMITIVE_RESTART);
}
}

////////////////////////////////////////////////// MyModel
namespace MyLoadedModel {


	GLuint modelShaders[2];
	GLuint modelProgram;



	const char* model_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	uniform vec3 lPos;\n\
	uniform vec3 lPos2;\n\
	uniform vec3 lPos3;\n\
	out vec3 lDir;\n\
	out vec3 lDir2;\n\
	out vec3 lDir3;\n\
	out vec4 vert_Normal;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
		lDir = normalize(lPos - (objMat * vec4(in_Position, 1.0)).xyz );\n\
		lDir2 = normalize(lPos2 - (objMat * vec4(in_Position, 1.0)).xyz );\n\
		lDir3 = normalize(lPos3 - (objMat * vec4(in_Position, 1.0)).xyz );\n\
	}";


	const char* model_fragShader =
		"#version 330\n\
		uniform int toonShading; \n\
		uniform int contourShading; \n\
		uniform int exCounter; \n\
		uniform int model; \n\
		in vec4 vert_Normal;\n\
		in vec3 lDir;\n\
		in vec3 lDir2;\n\
		in vec3 lDir3;\n\
		out vec4 out_Color;\n\
		uniform mat4 mv_Mat;\n\
		uniform vec4 modelcolor;\n\
		uniform vec4 color;\n\
		uniform vec4 color2;\n\
		uniform vec4 color3;\n\
		uniform vec4 ambient;\n\
		void main() {\n\
			if(exCounter==6){\n\
				if (toonShading == 3){ \n\
					float U=dot(vert_Normal, mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0));\n\
					float U2=dot(vert_Normal, mv_Mat*vec4(lDir2.x, lDir2.y, lDir2.z, 0.0));\n\
					float U3=dot(vert_Normal, mv_Mat*vec4(lDir3.x, lDir3.y, lDir3.z, 0.0));\n\
					out_Color = vec4(modelcolor.xyz, 1.0)*((vec4(color.xyz * U, 1.0 ) + vec4(color2.xyz * U2, 1.0) + vec4(color3.xyz * U3, 1.0))*ambient);\n\
				}\n\
				else if (toonShading == 0){\n\
					float U=dot(vert_Normal, mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0));\n\
					if(U<0.2) U=0.2;\n\
					else if(U>=0.2 && U<0.4) U=0.4;\n\
					else if(U>=0.4 && U<0.5) U=0.6;\n\
					else if(U>=0.5) U=1;\n\
					out_Color = vec4(modelcolor.xyz, 1.0)*((vec4(color.xyz * U, 1.0 )));\n\
				}\n\
				else if (toonShading == 1){\n\
					float U=dot(vert_Normal, mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0));\n\
					if(U<0.2) U=0.2;\n\
					else if(U>=0.2 && U<0.4) U=0.4;\n\
					else if(U>=0.4 && U<0.5) U=0.6;\n\
					else if(U>=0.5) U=1;\n\
					float U2=dot(vert_Normal, mv_Mat*vec4(lDir2.x, lDir2.y, lDir2.z, 0.0));\n\
					if(U2<0.2) U2=0.2;\n\
					else if(U2>=0.2 && U2<0.4) U2=0.4;\n\
					else if(U2>=0.4 && U2<0.5) U2=0.6;\n\
					else if(U2>=0.5) U2=1;\n\
					out_Color = vec4(modelcolor.xyz, 1.0)*((vec4(color.xyz * U, 1.0 ) + vec4(color2.xyz * U2, 1.0)));\n\
				}\n\
				else if (toonShading == 2){\n\
					float U2=dot(vert_Normal, mv_Mat*vec4(lDir2.x, lDir2.y, lDir2.z, 0.0));\n\
					if(U2<0.2) U2=0.2;\n\
					else if(U2>=0.2 && U2<0.4) U2=0.4;\n\
					else if(U2>=0.4 && U2<0.5) U2=0.6;\n\
					else if(U2>=0.5) U2=1;\n\
					float U3=dot(vert_Normal, mv_Mat*vec4(lDir3.x, lDir3.y, lDir3.z, 0.0));\n\
					if(U3<0.2) U3=0.2;\n\
					else if(U3>=0.2 && U3<0.4) U3=0.4;\n\
					else if(U3>=0.4 && U3<0.5) U3=0.6;\n\
					else if(U3>=0.5) U3=1;\n\
					out_Color = vec4(modelcolor.xyz, 1.0)*((vec4(color2.xyz * U2, 1.0) + vec4(color3.xyz * U3, 1.0))*ambient);\n\
				}\n\
				else{\n\
				float U=dot(vert_Normal, mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0));\n\
				if(U<0.2) U=0.2;\n\
				else if(U>=0.2 && U<0.4) U=0.4;\n\
				else if(U>=0.4 && U<0.5) U=0.6;\n\
				else if(U>=0.5) U=1;\n\
				\n\
				float U2=dot(vert_Normal, mv_Mat*vec4(lDir2.x, lDir2.y, lDir2.z, 0.0));\n\
				if(U2<0.2) U2=0.2;\n\
				else if(U2>=0.2 && U2<0.4) U2=0.4;\n\
				else if(U2>=0.4 && U2<0.5) U2=0.6;\n\
				else if(U2>=0.5) U2=1;\n\
				\n\
				float U3=dot(vert_Normal, mv_Mat*vec4(lDir3.x, lDir3.y, lDir3.z, 0.0));\n\
				if(U3<0.2) U3=0.2;\n\
				else if(U3>=0.2 && U3<0.4) U3=0.4;\n\
				else if(U3>=0.4 && U3<0.5) U3=0.6;\n\
				else if(U3>=0.5) U3=1;\n\
				\n\
				out_Color = vec4(modelcolor.xyz, 1.0)*((vec4(color.xyz * U, 1.0 ) + vec4(color2.xyz * U2, 1.0) + vec4(color3.xyz * U3, 1.0))*ambient);\n\
				\n\
				}\n\
			}\n\
			else if (exCounter==7){\n\
				switch(contourShading){\n\
				case 2:\n\
				case 0:{\n\
					float U = dot(vert_Normal, mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0)); \n\
					float U2 = dot(vert_Normal, mv_Mat*vec4(lDir2.x, lDir2.y, lDir2.z, 0.0)); \n\
					float U3 = dot(vert_Normal, mv_Mat*vec4(lDir3.x, lDir3.y, lDir3.z, 0.0)); \n\
					if(model<5)\n\
						out_Color = vec4(modelcolor.xyz, 1.0)*((vec4(color.xyz * U, 1.0) + vec4(color2.xyz * U2, 1.0) + vec4(color3.xyz * U3, 1.0))*ambient); \n\
					else\n\
						out_Color = vec4(modelcolor.xyz, 1.0); \n\
					break;}\n\
				case 1:{\n\
					float U=dot(vert_Normal, mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0));\n\
					if(U<0.2) U=0.2;\n\
					else if(U>=0.2 && U<0.4) U=0.4;\n\
					else if(U>=0.4 && U<0.5) U=0.6;\n\
					else if(U>=0.5) U=1;\n\
					\n\
					float U2=dot(vert_Normal, mv_Mat*vec4(lDir2.x, lDir2.y, lDir2.z, 0.0));\n\
					if(U2<0.2) U2=0.2;\n\
					else if(U2>=0.2 && U2<0.4) U2=0.4;\n\
					else if(U2>=0.4 && U2<0.5) U2=0.6;\n\
					else if(U2>=0.5) U2=1;\n\
					\n\
					float U3=dot(vert_Normal, mv_Mat*vec4(lDir3.x, lDir3.y, lDir3.z, 0.0));\n\
					if(U3<0.2) U3=0.2;\n\
					else if(U3>=0.2 && U3<0.4) U3=0.4;\n\
					else if(U3>=0.4 && U3<0.5) U3=0.6;\n\
					else if(U3>=0.5) U3=1;\n\
					\n\
					if(model<5)\n\
						out_Color = vec4(modelcolor.xyz, 1.0)*((vec4(color.xyz * U, 1.0 ) + vec4(color2.xyz * U2, 1.0) + vec4(color3.xyz * U3, 1.0))*ambient);\n\
					else\n\
						out_Color = vec4(modelcolor.xyz, 1.0); \n\
					break;}\n\
				}\n\
			}\n\
			else{\n\
				float U = dot(vert_Normal, mv_Mat*vec4(lDir.x, lDir.y, lDir.z, 0.0)); \n\
				float U2 = dot(vert_Normal, mv_Mat*vec4(lDir2.x, lDir2.y, lDir2.z, 0.0)); \n\
				float U3 = dot(vert_Normal, mv_Mat*vec4(lDir3.x, lDir3.y, lDir3.z, 0.0)); \n\
				out_Color = vec4(modelcolor.xyz, 1.0)*((vec4(color.xyz * U, 1.0) + vec4(color2.xyz * U2, 1.0) + vec4(color3.xyz * U3, 1.0))*ambient); \n\
			}\n\
		}";
	void setupModel(int model) {
		switch (model) {
		case 0:
			glGenVertexArrays(1, &trump::modelVao);
			glBindVertexArray(trump::modelVao);
			glGenBuffers(3, trump::modelVbo);

			glBindBuffer(GL_ARRAY_BUFFER, trump::modelVbo[0]);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, trump::modelVbo[1]);

			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			break;
		case 1:
			glGenVertexArrays(1, &chicken::modelVao);
			glBindVertexArray(chicken::modelVao);
			glGenBuffers(3, chicken::modelVbo);

			glBindBuffer(GL_ARRAY_BUFFER, chicken::modelVbo[0]);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, chicken::modelVbo[1]);

			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			break;
		case 2:
			for (int i = 0; i < shaders::nCabinas; ++i) {
				glGenVertexArrays(1, &cabina::modelVao[i]);
				glBindVertexArray(cabina::modelVao[i]);
				glGenBuffers(3, &cabina::modelVbo[i*3]);

				glBindBuffer(GL_ARRAY_BUFFER, cabina::modelVbo[i*3]);

				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
				glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(0);

				glBindBuffer(GL_ARRAY_BUFFER, cabina::modelVbo[i*3+1]);

				glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
				glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(1);
			}
			break;
		case 3:
			glGenVertexArrays(1, &pata::modelVao);
			glBindVertexArray(pata::modelVao);
			glGenBuffers(3, pata::modelVbo);

			glBindBuffer(GL_ARRAY_BUFFER, pata::modelVbo[0]);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, pata::modelVbo[1]);

			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			break;
		case 4:
			glGenVertexArrays(1, &noria::modelVao);
			glBindVertexArray(noria::modelVao);
			glGenBuffers(3, noria::modelVbo);

			glBindBuffer(GL_ARRAY_BUFFER, noria::modelVbo[0]);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, noria::modelVbo[1]);

			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			break;
		case 5:
			glGenVertexArrays(1, &contourTrump::modelVao);
			glBindVertexArray(contourTrump::modelVao);
			glGenBuffers(3, contourTrump::modelVbo);

			glBindBuffer(GL_ARRAY_BUFFER, contourTrump::modelVbo[0]);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, contourTrump::modelVbo[1]);

			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			break;
		case 6:
			glGenVertexArrays(1, &contourChicken::modelVao);
			glBindVertexArray(contourChicken::modelVao);
			glGenBuffers(3, contourChicken::modelVbo);

			glBindBuffer(GL_ARRAY_BUFFER, contourChicken::modelVbo[0]);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, contourChicken::modelVbo[1]);

			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			break;
		case 7:
			for (int i = 0; i < shaders::nCabinas; ++i) {
				glGenVertexArrays(1, &contourCabina::modelVao[i]);
				glBindVertexArray(contourCabina::modelVao[i]);
				glGenBuffers(3, &contourCabina::modelVbo[i * 3]);

				glBindBuffer(GL_ARRAY_BUFFER, contourCabina::modelVbo[i * 3]);

				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
				glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(0);

				glBindBuffer(GL_ARRAY_BUFFER, contourCabina::modelVbo[i * 3 + 1]);

				glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
				glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(1);
			}
			break;
		case 8:
			glGenVertexArrays(1, &contourPata::modelVao);
			glBindVertexArray(contourPata::modelVao);
			glGenBuffers(3, contourPata::modelVbo);

			glBindBuffer(GL_ARRAY_BUFFER, contourPata::modelVbo[0]);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, contourPata::modelVbo[1]);

			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			break;
		case 9:
			glGenVertexArrays(1, &contourNoria::modelVao);
			glBindVertexArray(contourNoria::modelVao);
			glGenBuffers(3, contourNoria::modelVbo);

			glBindBuffer(GL_ARRAY_BUFFER, contourNoria::modelVbo[0]);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, contourNoria::modelVbo[1]);

			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);
			break;
		}

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		modelShaders[0] = compileShader(model_vertShader, GL_VERTEX_SHADER, "cubeVert");
		modelShaders[1] = compileShader(model_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		modelProgram = glCreateProgram();
		glAttachShader(modelProgram, modelShaders[0]);
		glAttachShader(modelProgram, modelShaders[1]);
		glBindAttribLocation(modelProgram, 0, "in_Position");
		glBindAttribLocation(modelProgram, 1, "in_Normal");
		linkProgram(modelProgram);
	}
	void cleanupModel(int model) {
		switch (model) {
		case 0:
			glDeleteBuffers(2, trump::modelVbo);
			glDeleteVertexArrays(1, &trump::modelVao);
			break;
		case 1:
			glDeleteBuffers(2, chicken::modelVbo);
			glDeleteVertexArrays(1, &chicken::modelVao);
			break;
		case 2:
			for (int i = 0; i < shaders::nCabinas; ++i) {
				glDeleteBuffers(2, &cabina::modelVbo[i*3]);
				glDeleteVertexArrays(1, &cabina::modelVao[i]);
			}
			break;
		case 3:
			glDeleteBuffers(2, pata::modelVbo);
			glDeleteVertexArrays(1, &pata::modelVao);
			break;
		case 4:
			glDeleteBuffers(2, noria::modelVbo);
			glDeleteVertexArrays(1, &noria::modelVao);
			break;
		case 5:
			glDeleteBuffers(2, contourTrump::modelVbo);
			glDeleteVertexArrays(1, &contourTrump::modelVao);
			break;
		case 6:
			glDeleteBuffers(2, contourChicken::modelVbo);
			glDeleteVertexArrays(1, &contourChicken::modelVao);
			break;
		case 7:
			for (int i = 0; i < shaders::nCabinas; ++i) {
				glDeleteBuffers(2, &contourCabina::modelVbo[i * 3]);
				glDeleteVertexArrays(1, &contourCabina::modelVao[i]);
			}
			break;
		case 8:
			glDeleteBuffers(2, contourPata::modelVbo);
			glDeleteVertexArrays(1, &contourPata::modelVao);
			break;
		case 9:
			glDeleteBuffers(2, contourNoria::modelVbo);
			glDeleteVertexArrays(1, &contourNoria::modelVao);
			break;
		}

		glDeleteProgram(modelProgram);
		glDeleteShader(modelShaders[0]);
		glDeleteShader(modelShaders[1]);
	}
	void updateModel(const glm::mat4& transform, int model, int cabina) {
		switch (model) {
		case 0:
			trump::objMat = transform;
			break;
		case 1:
			chicken::objMat = transform;
			break;
		case 2:
			cabina::objMat[cabina] = transform;
			break;
		case 3:
			pata::objMat = transform;
			break;
		case 4:
			noria::objMat = transform;
			break;
		case 5:
			contourTrump::objMat = transform;
			break;
		case 6:
			contourChicken::objMat = transform;
			break;
		case 7:
			contourCabina::objMat[cabina] = transform;
			break;
		case 8:
			contourPata::objMat = transform;
			break;
		case 9:
			contourNoria::objMat = transform;
			break;
		}
	}
	void drawModel(int model, int cabina) {
		switch (model) {
		case 0:
			glBindVertexArray(trump::modelVao);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(trump::objMat));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), trump::color.x, trump::color.y, trump::color.z, trump::color.a);
			break;
		case 1:
			glBindVertexArray(chicken::modelVao);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(chicken::objMat));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), chicken::color.x, chicken::color.y, chicken::color.z, chicken::color.a);
			break;
		case 2:
			glBindVertexArray(cabina::modelVao[cabina]);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(cabina::objMat[cabina]));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), noria::color.x, noria::color.y, noria::color.z, noria::color.a);
			break;
		case 3:
			glBindVertexArray(pata::modelVao);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(pata::objMat));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), noria::color.x, noria::color.y, noria::color.z, noria::color.a);
			break;
		case 4:
			glBindVertexArray(noria::modelVao);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(noria::objMat));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), noria::color.x, noria::color.y, noria::color.z, noria::color.a);
			break;
		case 5:
			glBindVertexArray(contourTrump::modelVao);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(contourTrump::objMat));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), contourTrump::color.x, contourTrump::color.y, contourTrump::color.z, contourTrump::color.a);
			break;
		case 6:
			glBindVertexArray(contourChicken::modelVao);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(contourChicken::objMat));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), contourChicken::color.x, contourChicken::color.y, contourChicken::color.z, contourChicken::color.a);
			break;
		case 7:
			glBindVertexArray(contourCabina::modelVao[cabina]);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(contourCabina::objMat[cabina]));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), contourNoria::color.x, contourNoria::color.y, contourNoria::color.z, contourNoria::color.a);
			break;
		case 8:
			glBindVertexArray(contourPata::modelVao);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(contourPata::objMat));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), contourNoria::color.x, contourNoria::color.y, contourNoria::color.z, contourNoria::color.a);
			break;
		case 9:
			glBindVertexArray(contourNoria::modelVao);
			glUseProgram(modelProgram);
			glUniformMatrix4fv(glGetUniformLocation(modelProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(contourNoria::objMat));
			glUniform4f(glGetUniformLocation(modelProgram, "modelcolor"), contourNoria::color.x, contourNoria::color.y, contourNoria::color.z, contourNoria::color.a);
			break;
		}
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(modelProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform3f(glGetUniformLocation(modelProgram, "lPos"), sun::pos.x, sun::pos.y, sun::pos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "lPos2"), moon::pos.x, moon::pos.y, moon::pos.z);
		glUniform3f(glGetUniformLocation(modelProgram, "lPos3"), bulb::pos.x, bulb::pos.y, bulb::pos.z);
		glUniform4f(glGetUniformLocation(modelProgram, "color"), sun::color.x, sun::color.y, sun::color.z, sun::color.a);
		glUniform4f(glGetUniformLocation(modelProgram, "color2"), moon::color.x, moon::color.y, moon::color.z, moon::color.a);
		glUniform4f(glGetUniformLocation(modelProgram, "color3"), bulb::color.x, bulb::color.y, bulb::color.z, bulb::color.a);
		glUniform4f(glGetUniformLocation(modelProgram, "ambient"), sun::ambient.x, sun::ambient.y, sun::ambient.z, sun::ambient.a);
		glUniform1i(glGetUniformLocation(modelProgram, "toonShading"), GV::toonShading);
		glUniform1i(glGetUniformLocation(modelProgram, "contourShading"), GV::contourShading);
		glUniform1i(glGetUniformLocation(modelProgram, "exCounter"), GV::exCounter);
		glUniform1i(glGetUniformLocation(modelProgram, "model"), model);
	
		glDrawArrays(GL_TRIANGLES, 0, 25000);


		glUseProgram(0);
		glBindVertexArray(0);

	}


}


////////////////////////////////////////////////// CUBE
namespace Cube {
	GLuint cubeVao;
	GLuint cubeVbo[3];
	GLuint cubeShaders[2];
	GLuint cubeProgram;
	glm::mat4 objMat = glm::mat4(1.f);

	extern const float halfW = 0.5f;
	int numVerts = 24 + 6; // 4 vertex/face * 6 faces + 6 PRIMITIVE RESTART

						   //   4---------7
						   //  /|        /|
						   // / |       / |
						   //5---------6  |
						   //|  0------|--3
						   //| /       | /
						   //|/        |/
						   //1---------2
	glm::vec3 verts[] = {
		glm::vec3(-halfW, -halfW, -halfW),
		glm::vec3(-halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW, -halfW),
		glm::vec3(-halfW,  halfW, -halfW),
		glm::vec3(-halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW, -halfW)
	};
	glm::vec3 norms[] = {
		glm::vec3(0.f, -1.f,  0.f),
		glm::vec3(0.f,  1.f,  0.f),
		glm::vec3(-1.f,  0.f,  0.f),
		glm::vec3(1.f,  0.f,  0.f),
		glm::vec3(0.f,  0.f, -1.f),
		glm::vec3(0.f,  0.f,  1.f)
	};

	glm::vec3 cubeVerts[] = {
		verts[1], verts[0], verts[2], verts[3],
		verts[5], verts[6], verts[4], verts[7],
		verts[1], verts[5], verts[0], verts[4],
		verts[2], verts[3], verts[6], verts[7],
		verts[0], verts[4], verts[3], verts[7],
		verts[1], verts[2], verts[5], verts[6]
	};
	glm::vec3 cubeNorms[] = {
		norms[0], norms[0], norms[0], norms[0],
		norms[1], norms[1], norms[1], norms[1],
		norms[2], norms[2], norms[2], norms[2],
		norms[3], norms[3], norms[3], norms[3],
		norms[4], norms[4], norms[4], norms[4],
		norms[5], norms[5], norms[5], norms[5]
	};
	GLubyte cubeIdx[] = {
		0, 1, 2, 3, UCHAR_MAX,
		4, 5, 6, 7, UCHAR_MAX,
		8, 9, 10, 11, UCHAR_MAX,
		12, 13, 14, 15, UCHAR_MAX,
		16, 17, 18, 19, UCHAR_MAX,
		20, 21, 22, 23, UCHAR_MAX
	};




	const char* cube_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	out vec4 vert_Normal;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
	}";

	const char* cube_fragShader =
		"#version 330\n\
in vec4 vert_Normal;\n\
out vec4 out_Color;\n\
uniform mat4 mv_Mat;\n\
uniform vec4 color;\n\
void main() {\n\
	out_Color = vec4(color.xyz * dot(vert_Normal, mv_Mat*vec4(0.0, 1.0, 0.0, 0.0)) + color.xyz * 0.3, 1.0 );\n\
}";
	void setupCube() {
		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);
		glGenBuffers(3, cubeVbo);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNorms), cubeNorms, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glPrimitiveRestartIndex(UCHAR_MAX);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cubeShaders[0] = compileShader(cube_vertShader, GL_VERTEX_SHADER, "cubeVert");
		cubeShaders[1] = compileShader(cube_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		cubeProgram = glCreateProgram();
		glAttachShader(cubeProgram, cubeShaders[0]);
		glAttachShader(cubeProgram, cubeShaders[1]);
		glBindAttribLocation(cubeProgram, 0, "in_Position");
		glBindAttribLocation(cubeProgram, 1, "in_Normal");
		linkProgram(cubeProgram);
	}
	void cleanupCube() {
		glDeleteBuffers(3, cubeVbo);
		glDeleteVertexArrays(1, &cubeVao);

		glDeleteProgram(cubeProgram);
		glDeleteShader(cubeShaders[0]);
		glDeleteShader(cubeShaders[1]);
		glDeleteShader(cubeShaders[2]);
	}
	void updateCube(const glm::mat4& transform) {
		objMat = transform;
	}

	static float counter = 0;
	void drawCube(float currentTime) {

		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);

		//deltatime setup
		GV::dt = currentTime - GV::lastTime;

		//KEYBOARD INPUT
		const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
		if (keyboardState[SDL_SCANCODE_A]) { //next exercise
			if (!GV::pressed) {
				GV::pressed = true;
				if (GV::exCounter == nExercises)
					GV::exCounter = 1;
				else
					GV::exCounter++;
			}
		} 
		else if (keyboardState[SDL_SCANCODE_Z]) { //previous exercise
			if (!GV::pressed) {
				GV::pressed = true;
				if (GV::exCounter == 1)
					GV::exCounter = nExercises;
				else
					GV::exCounter--;
			}
		}
		else if (keyboardState[SDL_SCANCODE_M]) { //  models/cubes
			if (!GV::pressed) {
				GV::pressed = true;
				GV::models = !GV::models;
			}
		}
		else if (keyboardState[SDL_SCANCODE_C]) { //camera modes
			if (!GV::pressed) {
				GV::pressed = true;
				GV::cameraCounter++;
				GV::cameraCounter %= 4;
			}
		}
		else if (keyboardState[SDL_SCANCODE_D]) { //day/night transition
			if (!GV::pressed) {
				GV::pressed = true;
				light_moves = !light_moves;
			}
		}
		else if (keyboardState[SDL_SCANCODE_B]) { //bulb states
			if (!GV::pressed) {
				GV::pressed = true;
				GV::bulbState++;
				if (GV::bulbState == 3) {
					GV::bulbState = 0;
				}
			}
		}
		else if (keyboardState[SDL_SCANCODE_T]) { //toon shading
			if (GV::exCounter == 6) {
				if (!GV::pressed) {
					GV::pressed = true;
					GV::toonShading++;
					if (GV::toonShading == 4) {
						GV::toonShading = 0;
					}
				}
			}
			else if (GV::exCounter == 7) {
				if (!GV::pressed) {
					GV::pressed = true;
					GV::contourShading++;
					if (GV::contourShading == 3) {
						GV::contourShading = 0;
					}
				}
			}
		}
		else if (keyboardState[SDL_SCANCODE_S]) { //stencil buffer
			if (!GV::pressed) {
				GV::pressed = true;
				//logic here
			}
		}
		else
			GV::pressed = false;

		switch (GV::exCounter) {
		case 1: { //NORIA DE CUBOS
			RV::_modelView = glm::mat4(1);
			RV::_modelView = glm::translate(RV::_modelView, glm::vec3(0, -10, -80));
			RV::_modelView = glm::rotate(RV::_modelView, glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
			RV::_MVP = RV::_projection*RV::_modelView;
			
			for (int i = 0; i < shaders::nCabinas; ++i) {
					glm::vec3 posiciones = {	shaders::rCabinas * cos((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas),
												shaders::rCabinas * sin((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas), 0.f };
					glm::mat4 myObjMat = glm::translate(glm::mat4(1.0f), glm::vec3(posiciones.x, posiciones.y, posiciones.z));
					myObjMat *= glm::scale(glm::mat4(1.0f), glm::vec3(4, 4, 4));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(myObjMat));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
					glUniform1f(glGetUniformLocation(cubeProgram, "radius"), Cube::halfW * 20);
					glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
					glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
			}
		}
			break;
		case 2: { //NORIA DE MODELOS
			counter += GV::dt;
			RV::_modelView = glm::mat4(1);
			RV::_modelView = glm::translate(RV::_modelView, glm::vec3(0, -10, -80));
			RV::_modelView = glm::rotate(RV::_modelView, glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
			RV::_MVP = RV::_projection*RV::_modelView;
			

			for (int i = 0; i < shaders::nCabinas; ++i) {
				glm::vec3 posiciones = {	shaders::rCabinas * cos((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas),
											shaders::rCabinas * sin((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas), 0.f };
				glm::mat4 myObjMat = glm::translate(glm::mat4(1.0f), glm::vec3(posiciones.x, posiciones.y, posiciones.z));			


				if (GV::models) {

					//CABINAS
					MyLoadedModel::updateModel(myObjMat, 2, i);

					if (i == 0) {
						//TRUMP
						myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-1, -3, 0));
						myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 1, 0));
						MyLoadedModel::updateModel(myObjMat, 0);

						//GALLINA
						myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2));
						myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0, 1, 0));
						MyLoadedModel::updateModel(myObjMat, 1);

						//PIES NORIA
						glm::mat4 myNoriaMat = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
						MyLoadedModel::updateModel(myNoriaMat, 3);

						//RUEDA NORIA
						myNoriaMat = glm::rotate(glm::mat4(1.f), glm::radians(currentTime * 36), glm::vec3(0, 0, 1));
						MyLoadedModel::updateModel(myNoriaMat, 4);

					}
				}
				else {
					//CUBOS
					myObjMat *= glm::scale(glm::mat4(1.0f), glm::vec3(4, 4, 4));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(myObjMat));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
					glUniform1f(glGetUniformLocation(cubeProgram, "radius"), Cube::halfW * 20);
					glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
					glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
				}
			}
		}
			break;
		case 3: { //NORIA DE MODELOS
			counter += GV::dt;
			RV::_modelView = glm::mat4(1.f);
			RV::_modelView = glm::rotate(RV::_modelView, glm::radians(20.f), glm::vec3(1.f, 0.f, 0.f));


			for (int i = 0; i < shaders::nCabinas; ++i) {
				glm::vec3 posiciones = { shaders::rCabinas * cos((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas),
					shaders::rCabinas * sin((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas), 0.f };
				glm::mat4 myObjMat = glm::translate(glm::mat4(1.0f), glm::vec3(posiciones.x, posiciones.y, posiciones.z));

				//CÁMARA
				switch (GV::cameraCounter) {
				case 0://GENERAL SHOT
					RV::_modelView = glm::mat4(1.f);
					RV::_modelView = glm::translate(RV::_modelView, glm::vec3(7, 3, -60));
					RV::_modelView = glm::rotate(RV::_modelView, glm::radians(30.f), glm::vec3(0.f, 1.f, 0.f));
					RV::_MVP = RV::_projection*RV::_modelView;
					break;
				case 1://SHOT COUNTER SHOT
					if (i == shaders::nCabinas / 2) {
						if (counter < 2) {//POLLO
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(3, 2, 3));
							RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(65.f), glm::vec3(0, 1, 0));
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, 0));
						}
						else if (counter < 4) {//TRUMP
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(-3, 1, 3));
							RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(-75.f), glm::vec3(0, 1, 0));
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(-3, 0, 1));
						}
						else
							counter = 0;

						RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5)) * myObjMat;
						RV::_MVP = RV::_projection*RV::_modelView;
					}
					break;
				case 2://LATERAL VIEW
					RV::_modelView = glm::mat4(1.f);
					RV::_modelView = glm::translate(RV::_modelView, glm::vec3(0, 0, -70));
					RV::_MVP = RV::_projection*RV::_modelView;
					break;
				case 3://ROTATING GOD'S EYE SHOT
					if (i == shaders::nCabinas / 2) {

						RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(0, -1, -0.6));
						RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1, 0, 0));
						RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(currentTime * 50), glm::vec3(0, 1, 0));
						RV::_modelView *= myObjMat;

						RV::_MVP = RV::_projection*RV::_modelView;
					}
					break;
				}



				if (GV::models) {
					//CABINAS
					MyLoadedModel::updateModel(myObjMat, 2, i);

					if (i == 0) {
						//TRUMP
						myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-1, -3, 0));
						myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 1, 0));
						MyLoadedModel::updateModel(myObjMat, 0);

						//GALLINA
						myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2));
						myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0, 1, 0));
						MyLoadedModel::updateModel(myObjMat, 1);

						//PIES NORIA
						glm::mat4 myNoriaMat = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
						MyLoadedModel::updateModel(myNoriaMat, 3);

						//RUEDA NORIA
						myNoriaMat = glm::rotate(glm::mat4(1.f), glm::radians(currentTime * 36), glm::vec3(0, 0, 1));
						MyLoadedModel::updateModel(myNoriaMat, 4);
					}
				}


				else {
					//CUBOS
					myObjMat *= glm::scale(glm::mat4(1.0f), glm::vec3(4, 4, 4));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(myObjMat));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
					glUniform1f(glGetUniformLocation(cubeProgram, "radius"), Cube::halfW * 20);
					glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
					glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
				}


			}
		}
			break;
		case 4: { //LIGHT SOURCES
			counter += GV::dt;

			//CÁMARA
			RV::_modelView = glm::mat4(1);
			RV::_modelView = glm::translate(RV::_modelView, glm::vec3(0, -10, -80));
			RV::_modelView = glm::rotate(RV::_modelView, glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
			RV::_MVP = RV::_projection*RV::_modelView;


			for (int i = 0; i < shaders::nCabinas; ++i) {
				glm::vec3 posiciones = { shaders::rCabinas * cos((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas),
					shaders::rCabinas * sin((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas), 0.f };
				glm::mat4 myObjMat = glm::translate(glm::mat4(1.0f), glm::vec3(posiciones.x, posiciones.y, posiciones.z));

				if (GV::models) {

					//CABINAS
					MyLoadedModel::updateModel(myObjMat, 2, i);

					if (i == 0) {
						//BULB
						if ((GV::bulbState == 0 || GV::bulbState == 2)) {
							bulb::pos = myObjMat * glm::vec4(1);
						}

						//TRUMP
						myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-1, -3, 0));
						myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 1, 0));
						MyLoadedModel::updateModel(myObjMat, 0);

						//GALLINA
						myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2));
						myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0, 1, 0));
						MyLoadedModel::updateModel(myObjMat, 1);

						//PIES NORIA
						glm::mat4 myNoriaMat = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
						MyLoadedModel::updateModel(myNoriaMat, 3);

						//RUEDA NORIA
						myNoriaMat = glm::rotate(glm::mat4(1.f), glm::radians(currentTime * 36), glm::vec3(0, 0, 1));
						MyLoadedModel::updateModel(myNoriaMat, 4);

					}
				}


				else {
					//CUBOS
					myObjMat *= glm::scale(glm::mat4(1.0f), glm::vec3(4, 4, 4));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(myObjMat));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
					glUniform1f(glGetUniformLocation(cubeProgram, "radius"), Cube::halfW * 20);
					glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
					glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
				}


			}
		}
			break;
		case 5: { //NORIA DE MODELOS
			counter += GV::dt;

			for (int i = 0; i < shaders::nCabinas; ++i) {
				glm::vec3 posiciones = { shaders::rCabinas * cos((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas),
					shaders::rCabinas * sin((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas), 0.f };
				glm::mat4 myObjMat = glm::translate(glm::mat4(1.0f), glm::vec3(posiciones.x, posiciones.y, posiciones.z));

				if (i == 0 && (posiciones.y < -29.9f))
					GV::trumpNoria1 = false;
				
				//CÁMARA
				switch (GV::cameraCounter) {
				case 0://GENERAL SHOT
					RV::_modelView = glm::mat4(1.f);
					RV::_modelView = glm::translate(RV::_modelView, glm::vec3(-5, 3, -65));
					RV::_modelView = glm::rotate(RV::_modelView, glm::radians(30.f), glm::vec3(0.f, 1.f, 0.f));
					RV::_MVP = RV::_projection*RV::_modelView;

					break;
				case 1://SHOT COUNTER SHOT
					if (i == shaders::nCabinas / 2 && GV::trumpNoria1) {
						RV::_modelView = glm::mat4(1);
						if (counter < 2) {//POLLO
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(3, 2, 3));
							RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(65.f), glm::vec3(0, 1, 0));
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, 0));
						}
						else if (counter < 4) {//TRUMP
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(-3, 1, 3));
							RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(-75.f), glm::vec3(0, 1, 0));
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(-3, 0, 1));
						}
						else
							counter = 0;

						RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5));
						RV::_modelView *= myObjMat;
						RV::_MVP = RV::_projection*RV::_modelView;
					}
					break;
				case 2://LATERAL VIEW
					RV::_modelView = glm::mat4(1.f);
					RV::_modelView = glm::translate(RV::_modelView, glm::vec3(-32, 0, -80));
					RV::_MVP = RV::_projection*RV::_modelView;
					break;
				case 3://ROTATING GOD'S EYE SHOT
					if (i == shaders::nCabinas / 2 && GV::trumpNoria1) {

						RV::_modelView = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -0.6));
						RV::_modelView = glm::rotate(RV::_modelView, glm::radians(90.f), glm::vec3(1, 0, 0));
						RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(currentTime*50), glm::vec3(0, 1, 0));
						
						RV::_modelView *= myObjMat;

						RV::_MVP = RV::_projection*RV::_modelView;
					}
					break;
				}


				if (GV::models) {
					//CABINAS
					MyLoadedModel::updateModel(myObjMat, 2, i);

					if (i == 0) {
						if (GV::trumpNoria1) {
							//TRUMP
							myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-1, -3, 0));
							myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 1, 0));
							MyLoadedModel::updateModel(myObjMat, 0);

							//GALLINA
							myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2));
							myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0, 1, 0));
							MyLoadedModel::updateModel(myObjMat, 1);
						}
						//PIES NORIA
						glm::mat4 myNoriaMat = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
						MyLoadedModel::updateModel(myNoriaMat, 3);

						//RUEDA NORIA
						myNoriaMat = glm::rotate(glm::mat4(1.f), glm::radians(currentTime * 36), glm::vec3(0, 0, 1));
						MyLoadedModel::updateModel(myNoriaMat, 4);
					}
				}


				else {
					//CUBOS
					myObjMat *= glm::scale(glm::mat4(1.0f), glm::vec3(4, 4, 4));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(myObjMat));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
					glUniform1f(glGetUniformLocation(cubeProgram, "radius"), Cube::halfW * 20);
					glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
					glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
				}


			}
		}
			break;
		case 6: { //TOON SHADING
				counter += GV::dt;

				//FIX

				//CÁMARA
				RV::_modelView = glm::mat4(1);
				RV::_modelView = glm::translate(RV::_modelView, glm::vec3(0, -10, -80));
				RV::_modelView = glm::rotate(RV::_modelView, glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
				RV::_MVP = RV::_projection*RV::_modelView;


				for (int i = 0; i < shaders::nCabinas; ++i) {
					glm::vec3 posiciones = { shaders::rCabinas * cos((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas),
						shaders::rCabinas * sin((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas), 0.f };
					glm::mat4 myObjMat = glm::translate(glm::mat4(1.0f), glm::vec3(posiciones.x, posiciones.y, posiciones.z));

					if (GV::models) {
						//CABINAS
						MyLoadedModel::updateModel(myObjMat, 2, i);

						if (i == 0) {
							//TRUMP
							myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-1, -3, 0));
							myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 1, 0));
							MyLoadedModel::updateModel(myObjMat, 0);

							//GALLINA
							myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2));
							myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0, 1, 0));
							MyLoadedModel::updateModel(myObjMat, 1);

							//PIES NORIA
							glm::mat4 myNoriaMat = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
							MyLoadedModel::updateModel(myNoriaMat, 3);

							//RUEDA NORIA
							myNoriaMat = glm::rotate(glm::mat4(1.f), glm::radians(currentTime * 36), glm::vec3(0, 0, 1));
							MyLoadedModel::updateModel(myNoriaMat, 4);

						}
					}


					else {
						//CUBOS
						myObjMat *= glm::scale(glm::mat4(1.0f), glm::vec3(4, 4, 4));
						glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(myObjMat));
						glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
						glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
						glUniform1f(glGetUniformLocation(cubeProgram, "radius"), Cube::halfW * 20);
						glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
						glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
					}


				}
			}
			break;
		case 7: { //CONTOUR SHADING
			counter += GV::dt;
			RV::_modelView = glm::mat4(1.f);
			RV::_modelView = glm::rotate(RV::_modelView, glm::radians(20.f), glm::vec3(1.f, 0.f, 0.f));


			for (int i = 0; i < shaders::nCabinas; ++i) {
				glm::vec3 posiciones = { shaders::rCabinas * cos((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas),
					shaders::rCabinas * sin((float)2 * PI * 0.1* currentTime + 2 * PI * i / shaders::nCabinas), 0.f };
				glm::mat4 myObjMat = glm::translate(glm::mat4(1.0f), glm::vec3(posiciones.x, posiciones.y, posiciones.z));

				//CÁMARA
				switch (GV::cameraCounter) {
				case 0://GENERAL SHOT
					RV::_modelView = glm::mat4(1.f);
					RV::_modelView = glm::translate(RV::_modelView, glm::vec3(7, 3, -60));
					RV::_modelView = glm::rotate(RV::_modelView, glm::radians(30.f), glm::vec3(0.f, 1.f, 0.f));
					RV::_MVP = RV::_projection*RV::_modelView;
					break;
				case 1://SHOT COUNTER SHOT
					if (i == shaders::nCabinas / 2) {
						if (counter < 2) {//POLLO
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(3, 2, 3));
							RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(65.f), glm::vec3(0, 1, 0));
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, 0));
						}
						else if (counter < 4) {//TRUMP
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(-3, 1, 3));
							RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(-75.f), glm::vec3(0, 1, 0));
							RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(-3, 0, 1));
						}
						else
							counter = 0;

						RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5)) * myObjMat;
						RV::_MVP = RV::_projection*RV::_modelView;
					}
					break;
				case 2://LATERAL VIEW
					RV::_modelView = glm::mat4(1.f);
					RV::_modelView = glm::translate(RV::_modelView, glm::vec3(0, 0, -70));
					RV::_MVP = RV::_projection*RV::_modelView;
					break;
				case 3://ROTATING GOD'S EYE SHOT
					if (i == shaders::nCabinas / 2) {

						RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(0, -1, -0.6));
						RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1, 0, 0));
						RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(currentTime * 50), glm::vec3(0, 1, 0));
						RV::_modelView *= myObjMat;

						RV::_MVP = RV::_projection*RV::_modelView;
					}
					break;
				}



				if (GV::models) {
					//CABINAS
					MyLoadedModel::updateModel(myObjMat, 2, i);
					glm::mat4 contourNoria = glm::scale(myObjMat, glm::vec3(1.025f));
					MyLoadedModel::updateModel(contourNoria, 7, i);

					if (i == 0) {
						glm::vec4 tmpC = glm::vec4(0.f, 0.f, 0.1f, 1.f);
						//CONTOUR TRUMP
						myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-1, -3, 0));
						myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 1, 0));
						glm::mat4 contourTrump(1.f);
						if (GV::contourShading != 2) {
							contourTrump = glm::translate(myObjMat, glm::vec3(0, -.1, 0));
							contourTrump = glm::scale(contourTrump, glm::vec3(1.05f));
						}
						else {
							contourTrump = glm::translate(myObjMat, glm::vec3(0, -.2, 0));
							contourTrump = glm::scale(contourTrump, glm::vec3(1.1f));
						}
						MyLoadedModel::updateModel(contourTrump, 5);

						//TRUMP
						/*myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0.05));*/
						//myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 1, 0));
						MyLoadedModel::updateModel(myObjMat, 0);

						//GALLINA
						myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2));
						myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0, 1, 0));
						MyLoadedModel::updateModel(myObjMat, 1);
						glm::mat4 contourGallina;
						if (GV::contourShading != 2) {
							contourGallina = glm::translate(myObjMat, glm::vec3(0, -.1, 0));
							contourGallina = glm::scale(myObjMat, glm::vec3(1.05f));
						}
						else {
							contourGallina = glm::translate(myObjMat, glm::vec3(0, -.5, 0));
							contourGallina = glm::scale(myObjMat, glm::vec3(1.1f));
						}
						MyLoadedModel::updateModel(contourGallina, 6); //escalar

						//PIES NORIA
						glm::mat4 myNoriaMat = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
						MyLoadedModel::updateModel(myNoriaMat, 3);
						glm::mat4 contourPies = glm::scale(myNoriaMat, glm::vec3(1.005f));
						MyLoadedModel::updateModel(contourPies, 8);

						//RUEDA NORIA
						myNoriaMat = glm::rotate(glm::mat4(1.f), glm::radians(currentTime * 36), glm::vec3(0, 0, 1));
						MyLoadedModel::updateModel(myNoriaMat, 4);
						glm::mat4 contourNoria = glm::scale(myNoriaMat, glm::vec3(1.005f));
						MyLoadedModel::updateModel(contourNoria, 9);
					}
				}


				else {
					//CUBOS
					myObjMat *= glm::scale(glm::mat4(1.0f), glm::vec3(4, 4, 4));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(myObjMat));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
					glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
					glUniform1f(glGetUniformLocation(cubeProgram, "radius"), Cube::halfW * 20);
					glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
					glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
				}


			}
		}
				break;
		}		

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);

		GV::lastTime = currentTime;
	}


}


void noriaRelocateEx5(float currentTime, float offset) {

	//glm::mat4 myObjMat=glm::mat4(1);// = glm::translate(glm::mat4(1), glm::vec3(50, 0, 0));
	for (int i = 0; i < shaders::nCabinas; ++i) {
		glm::vec3 posiciones = { shaders::rCabinas * cos((float)2 * PI * 0.1* -currentTime + 2 * PI * i / shaders::nCabinas),
			shaders::rCabinas * sin((float)2 * PI * 0.1* -currentTime + 2 * PI * i / shaders::nCabinas), 0.f };
		glm::mat4 myObjMat = glm::translate(glm::mat4(1), glm::vec3(posiciones.x+ offset, posiciones.y, posiciones.z));

		//CÁMARA

		if (!GV::trumpNoria1) {
			switch (GV::cameraCounter) {
			case 1: //SHOT COUNTER SHOT
				if (i == shaders::nCabinas / 2) {
					RV::_modelView = glm::mat4(1);
					if (Cube::counter < 2) { //POLLO
						RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(3, 2, 3));
						RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(65.f), glm::vec3(0, 1, 0));
						RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, 0));
					}
					else if (Cube::counter < 4) { //TRUMP
						RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(-3, 1, 3));
						RV::_modelView *= glm::rotate(glm::mat4(1.f), glm::radians(-75.f), glm::vec3(0, 1, 0));
						RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(-3, 0, 1));
					}
					else
						Cube::counter = 0;

					RV::_modelView *= glm::translate(glm::mat4(1.0f), glm::vec3(-offset*2, 0, -5));
					RV::_modelView *= myObjMat;
					RV::_MVP = RV::_projection*RV::_modelView;

				}
				break;
			case 3: //ROTATING GOD'S EYE SHOT
				if (i == shaders::nCabinas / 2) {
					RV::_modelView = glm::mat4(1);

					RV::_modelView = glm::rotate(RV::_modelView, glm::radians(90.f), glm::vec3(1, 0, 0));
					RV::_modelView = glm::rotate(RV::_modelView, glm::radians(currentTime * 50), glm::vec3(0, 1, 0));
					RV::_modelView = glm::translate(RV::_modelView, glm::vec3(-2*offset, -0.6, 0));
					RV::_modelView *= myObjMat;					

					RV::_MVP = RV::_projection*RV::_modelView;
				}
				break;
			}
		}


		if (GV::models) {
			//CABINAS
			MyLoadedModel::updateModel(myObjMat, 2, i);

			if (i == 0) {

				glm::mat4 myNoriaMat = glm::translate(glm::mat4(1), glm::vec3(offset, 0, 0));
				//PIES NORIA
				MyLoadedModel::updateModel(myNoriaMat, 3);

				//RUEDA NORIA
				myNoriaMat = glm::rotate(myNoriaMat, glm::radians(-currentTime * 36), glm::vec3(0, 0, 1));
				
				MyLoadedModel::updateModel(myNoriaMat, 4);
			}

			if (i == 0) {
				if (!GV::trumpNoria1) {

					//TRUMP
					myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-1, -3, 0));
					myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 1, 0));
					MyLoadedModel::updateModel(myObjMat, 0);

					//GALLINA
					myObjMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2));
					myObjMat *= glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0, 1, 0));
					MyLoadedModel::updateModel(myObjMat, 1);

				}
				if (posiciones.y < -29.9)
					GV::trumpNoria1 = true;
			}
		}


		else {
			//CUBOS
			myObjMat *= glm::scale(glm::mat4(1.0f), glm::vec3(4, 4, 4));
			glUniformMatrix4fv(glGetUniformLocation(Cube::cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(myObjMat));
			glUniformMatrix4fv(glGetUniformLocation(Cube::cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
			glUniformMatrix4fv(glGetUniformLocation(Cube::cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
			glUniform1f(glGetUniformLocation(Cube::cubeProgram, "radius"), Cube::halfW * 20);
			glUniform4f(glGetUniformLocation(Cube::cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
			glDrawElements(GL_TRIANGLE_STRIP, Cube::numVerts, GL_UNSIGNED_BYTE, 0);
		}


	}
}