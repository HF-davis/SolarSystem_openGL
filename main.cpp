
#include <stdio.h>
#include <stdlib.h>
//#include <vlc/vlc.h>
#include <chrono>
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/SOIL2.h>
//#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <stack>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "ImportedModel.h"
#include "Sphere.h"
#include "Utils.h"
#include "camera.h"

using namespace std;


float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }

void mouse_callback(GLFWwindow* window,double xpos,double ypos);
void scroll_callback(GLFWwindow* window,double xoffset,double yoffset);
void processInput(GLFWwindow *window);


const unsigned int SCR_WIDTH=1200;
const unsigned int SCR_HEIGTH=1200;

#define numVAOs 1
#define numVBOs 9

//------camera position

float cameraX, cameraY, cameraZ;
float sphLocX, sphLocY, sphLocZ;

Camera camera(glm::vec3(0.0f, 0.0f, 150.0f));
float lastX=SCR_WIDTH/2.0f;
float lastY=SCR_HEIGTH/2.0f;

bool firstMouse=true;

//--------------timing

float deltaTime=0.0f;
float lastFrame=0.0f;

//---------------fin

GLuint renderingProgram,renderingProgram1,renderingProgramNormT,renderingProgramsec,renderingProgramCubeMap;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
GLuint earthTexture,sunTexture,mercuryTexture,venusTexture,marsTexture,jupiterTexture,saturnTexture,uranusTexture,neptuneTexture;
GLuint astTexture,earthMoonTexture,moonNormalMap;
GLuint sky_box_stars;

// variable allocation for display
GLuint mvLoc, projLoc,vLoc,tfLoc,nLoc;
int width, height;
float aspect,timeFactor;
glm::mat4 pMat, vMat, mMat, mvMat,view,invTrMat,rMat;
stack<glm::mat4> mvStack;



ImportedModel myModel("asteroid/as.obj");
Sphere mySphere = Sphere(48);


//lights variables
float lightLocX, lightLocY, lightLocZ;
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;
glm::vec3 currentLightPos;
glm::vec3 lightLoc=glm::vec3(50.0f,20.0f,20.0f);
float lightPos[3];
float amt = 0.0f;

// white light
float globalAmbient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
float lightAmbient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// silver material
float* matAmb = Utils::silverAmbient();
float* matDif = Utils::silverDiffuse();
float* matSpe = Utils::silverSpecular();
float matShi = Utils::silverShininess();

/*
void playMusic(void){

	libvlc_instance_t *inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;
	
	inst=libvlc_new(0,NULL);
	
	m=libvlc_media_new_path(inst,"music/pa.mp3");
	
	mp=libvlc_media_player_new_from_media(m);
	
	libvlc_media_player_play(mp);
	
	//Sleep(10);
	std::this_thread::sleep_for(std::chrono::milliseconds(240000));
	
	libvlc_media_player_stop(mp);
	
	libvlc_release(inst);
	
}*/
void installLights(int renderingProgramLights,glm::mat4 v_matrix){
	glm::vec3 transformed = glm::vec3(v_matrix * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = transformed.x;
	lightPos[1] = transformed.y;
	lightPos[2] = transformed.z;

	globalAmbLoc = glGetUniformLocation(renderingProgramLights, "globalAmbient");
	ambLoc = glGetUniformLocation(renderingProgramLights, "light.ambient");
	diffLoc = glGetUniformLocation(renderingProgramLights, "light.diffuse");
	specLoc = glGetUniformLocation(renderingProgramLights, "light.specular");
	posLoc = glGetUniformLocation(renderingProgramLights, "light.position");
	mambLoc = glGetUniformLocation(renderingProgramLights, "material.ambient");
	mdiffLoc = glGetUniformLocation(renderingProgramLights, "material.diffuse");
	mspecLoc = glGetUniformLocation(renderingProgramLights, "material.specular");
	mshiLoc = glGetUniformLocation(renderingProgramLights, "material.shininess");

	glProgramUniform4fv(renderingProgramLights, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgramLights, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(renderingProgramLights, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(renderingProgramLights, specLoc, 1, lightSpecular);
	glProgramUniform3fv(renderingProgramLights, posLoc, 1, lightPos);
	glProgramUniform4fv(renderingProgramLights, mambLoc, 1, matAmb);
	glProgramUniform4fv(renderingProgramLights, mdiffLoc, 1, matDif);
	glProgramUniform4fv(renderingProgramLights, mspecLoc, 1, matSpe);
	glProgramUniform1f(renderingProgramLights, mshiLoc, matShi);

}


void setupVertices(void) {
	std::vector<int> indp = mySphere.getIndices();
	std::vector<glm::vec3> vertp = mySphere.getVertices();
	std::vector<glm::vec2> texp = mySphere.getTexCoords();
	std::vector<glm::vec3> normp = mySphere.getNormals();
	std::vector<glm::vec3> tang = mySphere.getTangents();


	std::vector<float> pvaluesp;
	std::vector<float> tvaluesp;
	std::vector<float> nvaluesp;
	std::vector<float> tanvalues;

	int numIndices = mySphere.getNumIndices();
	for (int i = 0; i < numIndices; i++) {
		pvaluesp.push_back((vertp[indp[i]]).x);
		pvaluesp.push_back((vertp[indp[i]]).y);
		pvaluesp.push_back((vertp[indp[i]]).z);
		tvaluesp.push_back((texp[indp[i]]).s);
		tvaluesp.push_back((texp[indp[i]]).t);
		nvaluesp.push_back((normp[indp[i]]).x);
		nvaluesp.push_back((normp[indp[i]]).y);
		nvaluesp.push_back((normp[indp[i]]).z);
		tanvalues.push_back((tang[indp[i]]).x);
		tanvalues.push_back((tang[indp[i]]).y);
		tanvalues.push_back((tang[indp[i]]).z);
	}

	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pvaluesp.size()*4, &pvaluesp[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tvaluesp.size()*4, &tvaluesp[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nvaluesp.size()*4, &nvaluesp[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, tanvalues.size() * 4, &tanvalues[0], GL_STATIC_DRAW);



	///--------------------------------vert asteroid

	std::vector<glm::vec3> vert = myModel.getVertices();
	std::vector<glm::vec2> tex = myModel.getTextureCoords();
	std::vector<glm::vec3> norm = myModel.getNormals();

	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;

	for (int i = 0; i < myModel.getNumVertices(); i++) {
		pvalues.push_back((vert[i]).x);
		pvalues.push_back((vert[i]).y);
		pvalues.push_back((vert[i]).z);
		tvalues.push_back((tex[i]).s);
		tvalues.push_back((tex[i]).t);
		nvalues.push_back((norm[i]).x);
		nvalues.push_back((norm[i]).y);
		nvalues.push_back((norm[i]).z);


	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 2, &tvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);


	//-------------------------vert cubemap

		float cubeVertexPositions[108] =
	{	-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
	};
	

	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);



}


void setupTextures(void){
	//----------------------planets texture
	sunTexture=Utils::loadTexture("textures/8k_sun.jpg");
	mercuryTexture=Utils::loadTexture("textures/8k_mercury.jpg");
	venusTexture=Utils::loadTexture("textures/8k_venus_surface.jpg");
	earthTexture = Utils::loadTexture("textures/8k_earth_daymap.jpg");
	marsTexture=Utils::loadTexture("textures/8k_mars.jpg");
	jupiterTexture=Utils::loadTexture("textures/8k_jupiter.jpg");
	saturnTexture=Utils::loadTexture("textures/8k_saturn.jpg");
	uranusTexture=Utils::loadTexture("textures/2k_uranus.jpg");
	neptuneTexture=Utils::loadTexture("textures/2k_neptune.jpg");

	//------------moon texture

	earthMoonTexture=Utils::loadTexture("textures/moon.jpg");
	moonNormalMap = Utils::loadTexture("textures/moonNORMAL.jpg");

	//--------------asteroid texture
	astTexture=Utils::loadTexture("asteroid/rock.png");

	//-------------------skySTARS
	sky_box_stars=Utils::loadCubeMap2("cubeMap");
}

void init(GLFWwindow* window) {
	renderingProgram = Utils::createShaderProgram("shaders/vertShader.glsl", "shaders/fragShader.glsl");
	renderingProgram1=Utils::createShaderProgram("shaders/vertShaderast.glsl","shaders/fragShaderast.glsl");
	renderingProgramsec=Utils::createShaderProgram("shaders/vertShadersec.glsl","shaders/fragShadersec.glsl");
	renderingProgramNormT=Utils::createShaderProgram("shaders/vertShaderNormT.glsl","shaders/fragShaderNormT.glsl");
	renderingProgramCubeMap=Utils::createShaderProgram("shaders/vertShaderCubeMap.glsl","shaders/fragShaderCubeMap.glsl");

	cameraX = 0.0f; cameraY = 0.0f; cameraZ = 150.0f;
	sphLocX = 0.0f; sphLocY = 0.0f; sphLocZ = 0.0f;
	lightLocX = 3.0f; lightLocY = 2.0f; lightLocZ = 3.0f;

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);

	setupVertices();
	setupTextures();
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	
}

void display(GLFWwindow* window, double currentTime) {
	glClear(GL_DEPTH_BUFFER_BIT);
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);


	//para ver el movimiento circulatorio desde el eje y
	//view = glm::lookAt(glm::vec3(0.0f, 80.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	//vMat=view;

	//put cube map
	glUseProgram(renderingProgramCubeMap);

	vLoc = glGetUniformLocation(renderingProgramCubeMap, "v_matrix");
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));

	projLoc = glGetUniformLocation(renderingProgramCubeMap, "p_matrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, sky_box_stars);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);	// cube is CW, but we are viewing the inside
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glEnable(GL_DEPTH_TEST);

	
	///-----------------------------draw solarsystem
	glUseProgram(renderingProgram);
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	nLoc= glGetUniformLocation(renderingProgram, "norm_matrix");

	
	mvStack.push(vMat);

	currentLightPos = glm::vec3(lightLoc.x, lightLoc.y, lightLoc.z);
	amt*=2.0f;
	//rMat=glm::translate(glm::mat4(1.0f),glm::vec3(cos((float)currentTime)*10.0,sin((float)currentTime)*10.0,0.0f));
	//rMat = glm::rotate(glm::mat4(1.0f), toRadians(amt), glm::vec3(0.0f, 0.0f, 1.0f));
	//currentLightPos = glm::vec3(rMat * glm::vec4(currentLightPos, 1.0f));


	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));


	installLights(renderingProgram,vMat);

	//	installLights(renderingProgramNormT,vMat);
	//mMat = glm::translate(glm::mat4(1.0f), glm::vec3(sphLocX, sphLocY, sphLocZ));
	
	//--------------------------------------sun
	mvStack.push(mvStack.top());
	
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));  // sun position
        mvStack.push(mvStack.top()); 
   	mMat=glm::scale(glm::mat4(1.0f),glm::vec3(10.0f,10.0f,10.0f));//original is 10
	mMat = glm::rotate(mMat,(float)currentTime ,glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() *=mMat;
	//mvMat = vMat * mMat;

	
	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sunTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
	mvStack.pop();
	mvStack.pop();

	//-----------------cinturon asteroides

	glUseProgram(renderingProgramsec);
	vLoc = glGetUniformLocation(renderingProgramsec, "v_matrix");
	projLoc = glGetUniformLocation(renderingProgramsec, "proj_matrix");

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));


	timeFactor=((float)currentTime);
    tfLoc=glGetUniformLocation(renderingProgramsec,"tf");
    glUniform1f(tfLoc,(float)timeFactor);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, astTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

     glDrawArraysInstanced(GL_TRIANGLES, 0, myModel.getNumVertices(),400);
	//-----------------------------mercury
     glUseProgram(renderingProgram);
	mvStack.push(mvStack.top());
	//mMat = glm::rotate(glm::mat4(1.0f),185.0f ,glm::vec3(0.0f, 0.0f, 1.0f));
	mvStack.top()*=glm::translate(glm::mat4(1.0f),glm::vec3(cos((float)currentTime)*16.0,0.0f,sin((float)currentTime)*16.0));
	mvStack.push(mvStack.top());
	mvStack.top() *=scale(glm::mat4(1.0f),glm::vec3(0.2f,0.2f,0.2f));
	//mvStack.top() *=scale(glm::mat4(1.0f),glm::vec3(1.0f,1.0f,1.0f));
	mvStack.top()*=rotate(glm::mat4(1.0f),(float)currentTime,glm::vec3(0.0,1.0,0.0));

	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mercuryTexture);

	glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
	mvStack.pop();
	mvStack.pop();
	
	//--------------------------------------------------------------venus
	mvStack.push(mvStack.top());
	//mMat = glm::rotate(glm::mat4(1.0f),185.0f ,glm::vec3(0.0f, 0.0f, 1.0f));
	mvStack.top()*=glm::translate(glm::mat4(1.0f),glm::vec3(cos((float)currentTime/2.6)*20.0,0.0f,sin((float)currentTime/2.6)*20.0));
	mvStack.push(mvStack.top());
	mvStack.top() *=scale(glm::mat4(1.0f),glm::vec3(0.6f,0.6f,0.6f));
	mvStack.top()*=rotate(glm::mat4(1.0f),(float)currentTime,glm::vec3(0.0,1.0,0.0));

	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, venusTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
	mvStack.pop();
	mvStack.pop();
	//------------------------------earth
	mvStack.push(mvStack.top());
	//mMat = glm::rotate(glm::mat4(1.0f),185.0f ,glm::vec3(0.0f, 0.0f, 1.0f));
	
	mvStack.top()*=glm::translate(glm::mat4(1.0f),glm::vec3(cos((float)currentTime/4.17)*24.0,0.0f,sin((float)currentTime/4.17)*24.0));
	mvStack.push(mvStack.top());
	mvStack.top() *=scale(glm::mat4(1.0f),glm::vec3(0.64f,0.64f,0.64f));
	mvStack.top()*=rotate(glm::mat4(1.0f),(float)currentTime,glm::vec3(0.0,1.0,0.0));

	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
	mvStack.pop();
	

	//------------------------------earthMoon
	
	//glUseProgram(renderingProgramNormT);
	//mvLoc = glGetUniformLocation(renderingProgramNormT, "mv_matrix");
	//projLoc = glGetUniformLocation(renderingProgramNormT, "proj_matrix");
	//nLoc= glGetUniformLocation(renderingProgramNormT, "norm_matrix");

	mvStack.push(mvStack.top());
	mvStack.top()*=glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,sin((float)currentTime)*2.0,cos((float)currentTime)*2.0));
	mvStack.top()*=scale(glm::mat4(1.0f),glm::vec3(0.1f,0.1f,0.1f));
	mvStack.top()*=rotate(glm::mat4(1.0f),(float)currentTime,glm::vec3(0.0,0.0,1.0));

	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthMoonTexture);
	

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(2);

	//glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(3);


	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, moonNormalMap);

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, earthMoonTexture);
	
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

	mvStack.pop();
	mvStack.pop();
	//------------------------------------mars
	glUseProgram(renderingProgram);
	mvStack.push(mvStack.top());
	//mMat = glm::rotate(glm::mat4(1.0f),185.0f ,glm::vec3(0.0f, 0.0f, 1.0f));
	
	mvStack.top()*=glm::translate(glm::mat4(1.0f),glm::vec3(cos((float)currentTime/7.83)*28.0,0.0f,sin((float)currentTime/7.83)*28.0));
	mvStack.push(mvStack.top());
	mvStack.top() *=scale(glm::mat4(1.0f),glm::vec3(0.3f,0.3f,0.3f));
	mvStack.top()*=rotate(glm::mat4(1.0f),(float)currentTime,glm::vec3(0.0,1.0,0.0));

	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, marsTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
	mvStack.pop();
	mvStack.pop();
	//-------------------------------------------------------------jupiter
	mvStack.push(mvStack.top());
	//mMat = glm::rotate(glm::mat4(1.0f),185.0f ,glm::vec3(0.0f, 0.0f, 1.0f));

	mvStack.top()*=glm::translate(glm::mat4(1.0f),glm::vec3(cos((float)currentTime/49.16)*38.0,0.0f,sin((float)currentTime/49.16)*38.0));
	mvStack.push(mvStack.top());
	mvStack.top() *=scale(glm::mat4(1.0f),glm::vec3(5.0f,5.0f,5.0f));
	mvStack.top()*=rotate(glm::mat4(1.0f),(float)currentTime,glm::vec3(0.0,1.0,0.0));

	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, jupiterTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
	mvStack.pop();
	mvStack.pop();
	//_-----------------------------------------------------saturn
	mvStack.push(mvStack.top());
	//mMat = glm::rotate(glm::mat4(1.0f),185.0f ,glm::vec3(0.0f, 0.0f, 1.0f));
	
	mvStack.top()*=glm::translate(glm::mat4(1.0f),glm::vec3(cos((float)currentTime/122.9)*50.0,0.0f,sin((float)currentTime/122.9)*50.0));
	mvStack.push(mvStack.top());
	mvStack.top() *=scale(glm::mat4(1.0f),glm::vec3(4.0f,4.0f,4.0f));
	mvStack.top()*=rotate(glm::mat4(1.0f),(float)currentTime,glm::vec3(0.0,1.0,0.0));

	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, saturnTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
	mvStack.pop();
	//mvStack.pop();

	///-----------------------asteroid rings

	

	glUseProgram(renderingProgram1);
	vLoc = glGetUniformLocation(renderingProgram1, "v_matrix");
	projLoc = glGetUniformLocation(renderingProgram1, "proj_matrix");

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));


	timeFactor=((float)currentTime);
    tfLoc=glGetUniformLocation(renderingProgram1,"tf");
    glUniform1f(tfLoc,(float)timeFactor);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, astTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

     glDrawArraysInstanced(GL_TRIANGLES, 0, myModel.getNumVertices(),200);

     mvStack.pop();

	//---------------------------------------------------uranus

	glUseProgram(renderingProgram);

	mvStack.push(mvStack.top());
	//mMat = glm::rotate(glm::mat4(1.0f),185.0f ,glm::vec3(0.0f, 0.0f, 1.0f));
	
	mvStack.top()*=glm::translate(glm::mat4(1.0f),glm::vec3(cos((float)currentTime/350)*62.0,0.0f,sin((float)currentTime/350)*62.0));
	mvStack.push(mvStack.top());
	mvStack.top() *=scale(glm::mat4(1.0f),glm::vec3(2.5f,2.5f,2.5f));
	mvStack.top()*=rotate(glm::mat4(1.0f),(float)currentTime,glm::vec3(0.0,1.0,0.0));

	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, uranusTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
	mvStack.pop();
	mvStack.pop();
	//-------------------------------neptune
	mvStack.push(mvStack.top());
	//mMat = glm::rotate(glm::mat4(1.0f),185.0f ,glm::vec3(0.0f, 0.0f, 1.0f));
	
	mvStack.top()*=glm::translate(glm::mat4(1.0f),glm::vec3(cos((float)currentTime/686)*74.0,0.0f,sin((float)currentTime/686)*74.0));
	mvStack.push(mvStack.top());
	mvStack.top() *=scale(glm::mat4(1.0f),glm::vec3(2.4f,2.4f,2.4f));
	mvStack.top()*=rotate(glm::mat4(1.0f),(float)currentTime,glm::vec3(0.0,1.0,0.0));

	invTrMat=glm::transpose(glm::inverse(mvStack.top()));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, neptuneTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());
	mvStack.pop();
	mvStack.pop();
	mvStack.pop();
	
	

}

void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
	aspect = (float)newWidth / (float)newHeight;
	glViewport(0, 0, newWidth, newHeight);
	//pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGTH, "SolarSystem", NULL, NULL);
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window,window_size_callback);
	glfwSetCursorPosCallback(window,mouse_callback);
	glfwSetScrollCallback(window,scroll_callback);

	glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, window_size_callback);

	init(window);
	//playMusic();
	while (!glfwWindowShouldClose(window)) {

		float currentFrame=glfwGetTime();
		deltaTime=currentFrame-lastFrame;
		lastFrame=currentFrame;

		processInput(window);
		pMat=glm::perspective(glm::radians(camera.Zoom),(float)SCR_WIDTH/(float)SCR_HEIGTH,0.1f,1000.0f);
		vMat=camera.GetViewMatrix();

		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window,double xpos,double ypos)
{
	if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}