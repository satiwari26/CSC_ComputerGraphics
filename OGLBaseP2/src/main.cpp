/*
 * Example two meshes and two shaders (could also be used for Program 2)
 * includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "Bezier.h"
#include "Spline.h"
#include "particleSys.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#define PI 3.1415927

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;


class Application : public EventCallbacks {

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Our shader program for particles
	std::shared_ptr<Program> partProg;

	//the partricle system
	particleSys *thePartSystem;
	particleSys *thePartSystem2;

	// OpenGL handle to texture data used in particle
	shared_ptr<Texture> texture;

	// Our shader program
	std::shared_ptr<Program> solidColorProg;

	//for lightning effect
	int lightCounter = 0;

	//Our shader program for textures
	std::shared_ptr<Program> texProg;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> mesh;

	//a different mesh
	shared_ptr<Shape> bunny;

	//tree base
	shared_ptr<Shape> tree;
	vector<shared_ptr<Shape>> trees;

	//car base
	shared_ptr<Shape> car;
	vector<shared_ptr<Shape>> cars;

	//building
	shared_ptr<Shape> building;
	vector<shared_ptr<Shape>> buildings;

	//background skybox
	shared_ptr<Shape> cube;
	vector<shared_ptr<Shape>> cubes;
	//texture for the skybox
	shared_ptr<Texture> skyBoxTexture;
	shared_ptr<Texture> skyBoxTexture2;

	//batman
	shared_ptr<Shape> batman;
	vector<shared_ptr<Shape>> batmans;
	vector<tinyobj::shape_t> TOshapes6;
	vector<tinyobj::material_t> objMaterials2;

	//ground
	shared_ptr<Shape> ground;
	vector<tinyobj::shape_t> TOshapesGround;
	shared_ptr<Texture> groundTexture;

	//grass
	shared_ptr<Shape> grass;
	vector<shared_ptr<Shape>> grasses;

	//Mountain
	shared_ptr<Shape> mountain;
	vector<shared_ptr<Shape>> mountains;

	//create texture for the world
	shared_ptr<Texture> texture0;

	//city
	shared_ptr<Shape> city;
	vector<shared_ptr<Shape>> cities;
	vector<tinyobj::shape_t> TOshapes9;
	vector<tinyobj::material_t> objMaterials1;

	bool switchShader = false;

	//batarang
	shared_ptr<Shape> m4a1;
	vector<shared_ptr<Shape>> m4a1s;

	//bricks
	shared_ptr<Shape> brick;
	vector<shared_ptr<Shape>> bricks;

	//batarang
	shared_ptr<Shape> brick1;
	vector<shared_ptr<Shape>> bricks1;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	//animation data
	float sTheta = 0;
	float gTrans = -6;
	float gTrans2 = 0;
	float gLight = 0;
	float gMove = 0;

	//camera
	double g_phi, g_theta;
	vec3 view = vec3(0, 0, 1);
	vec3 strafe = vec3(1, 0, 0);
	vec3 g_eye = vec3(10.0, 2.2, 25);
	vec3 g_lookAt = vec3(7.0, 2.2, -1);
	vec3 gaze = g_lookAt - g_eye;
	vec3 g_up = vec3(0, 1, 0);

	// These variables store the cursor position in the previous frame
	double lastX = 0.0, lastY = 0.0;
	float yaw = 0.0f, pitch = 0.0f;

	vec3 initialEye = vec3(10.0, 2.2, 25);
	vec3 initialLookAt = vec3(7.0, 2.2, -1);

	Spline splinepath[2];
	bool goCamera = false;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		float cameraSpeed = 0.5f;

    // w, u, v vectors
    glm::vec3 w = glm::normalize(g_eye - g_lookAt);
    glm::vec3 u = glm::normalize(glm::cross(vec3(0, 1, 0), w));
    glm::vec3 v = glm::cross(w, u);

		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			if (key == GLFW_KEY_W) {
				g_eye -= cameraSpeed * w;
				g_lookAt -= cameraSpeed * w;
			}
			if (key == GLFW_KEY_S) {
				g_eye += cameraSpeed * w;
				g_lookAt += cameraSpeed * w;
			}
			if (key == GLFW_KEY_A) {
				g_eye -= cameraSpeed * u;
				g_lookAt -= cameraSpeed * u;
			}
			if (key == GLFW_KEY_D) {
				g_eye += cameraSpeed * u;
				g_lookAt += cameraSpeed * u;
			}
		}

		if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			gLight -= 0.2;
		}
		if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			gLight += 0.2;
		}
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			gTrans2 -= 0.2;
		}
		if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			gTrans2 += 0.2;
		}
		if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			sTheta -= 0.2;
		}
		if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		// if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		// 	gMove += 0.2;
		// }
		// if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		// 	gMove -= 0.2;
		// }
		if (key == GLFW_KEY_M && (action == GLFW_PRESS)) {
			switchShader = !switchShader;
		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) {
			goCamera = !goCamera;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		// delta x and y position
		double xoffset = xpos - lastX;
		double yoffset = lastY - ypos; 

		// getting current cursor position
		lastX = xpos;
		lastY = ypos;

		// mouse sensitivity
		float sensitivity = 0.2f;

		yaw   += xoffset * sensitivity;
		pitch += yoffset * sensitivity;

		// screen flipping issue 
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		// directional vector
		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		glm::vec3 w = glm::normalize(direction);

		// u and v vectors
		glm::vec3 u = glm::normalize(glm::cross(vec3(0, 1, 0), w));
		glm::vec3 v = glm::cross(w, u);

		// Calculate the new lookAt point
		g_lookAt = g_eye + w;
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
   		// cout << "xDel + yDel " << deltaX << " " << deltaY << endl;
   		// //fill in for game camera

		
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);

		// Enable z-buffer test.
		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(50.0f));

		// Initialize the GLSL program.
		partProg = make_shared<Program>();
		partProg->setVerbose(true);
		partProg->setShaderNames(
			resourceDirectory + "/lab10_vert.glsl",
			resourceDirectory + "/lab10_frag.glsl");
		if (! partProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		partProg->addUniform("P");
		partProg->addUniform("M");
		partProg->addUniform("V");
		partProg->addUniform("pColor");
		partProg->addUniform("alphaTexture");
		partProg->addAttribute("vertPos");

		thePartSystem = new particleSys(vec3(7.0, 60, -1));
		thePartSystem->gpuSetup();
		thePartSystem2 = new particleSys(vec3(7.0, 60, -1));
		thePartSystem2->gpuSetup();


		g_theta = -PI/2.0;

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
		prog->addUniform("MatDif");
		prog->addUniform("lightPos");
		prog->addUniform("lightIntensity");
		prog->addUniform("lightPos2");
		prog->addUniform("lightIntensity2");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

		// Initialize the GLSL program.
		solidColorProg = make_shared<Program>();
		solidColorProg->setVerbose(true);
		solidColorProg->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/solid_frag.glsl");
		solidColorProg->init();
		solidColorProg->addUniform("P");
		solidColorProg->addUniform("V");
		solidColorProg->addUniform("M");
		solidColorProg->addUniform("solidColor");
		solidColorProg->addAttribute("vertPos");
		solidColorProg->addAttribute("vertNor");

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();	
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("Texture0");
		texProg->addUniform("lightPos");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");

		//read in a load the texture
		texture0 = make_shared<Texture>();
  		texture0->setFilename(resourceDirectory + "/grass.jpg");
  		texture0->init();
  		texture0->setUnit(0);
  		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		//sky box texture
		skyBoxTexture = make_shared<Texture>();
		skyBoxTexture->setFilename(resourceDirectory + "/boundingBox1.jpg");
		skyBoxTexture->init();
		skyBoxTexture->setUnit(0);
		skyBoxTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		//sky box texture2
		skyBoxTexture2 = make_shared<Texture>();
		skyBoxTexture2->setFilename(resourceDirectory + "/boundingBox2.jpg");
		skyBoxTexture2->init();
		skyBoxTexture2->setUnit(0);
		skyBoxTexture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		//ground texture
		groundTexture = make_shared<Texture>();
		groundTexture->setFilename(resourceDirectory + "/boundingBox2.jpg");
		groundTexture->init();
		groundTexture->setUnit(0);
		groundTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		float bx = 7.0;
		float by = 6.2;
		float bz = -1.0;

		// Move towards Batman
		splinepath[0] = Spline(glm::vec3(-2,0,7), glm::vec3(1,5,20), glm::vec3(bx + 7, by+11, bz -30), glm::vec3(bx-3, by, bz + 20), 15);
	}

	void initGeom(const std::string& resourceDirectory)
	{

		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/SmoothSphere.obj").c_str());
		
		if (!rc) {
			cerr << errStr << endl;
		} else {
			//for now all our shapes will not have textures - change in later labs
			mesh = make_shared<Shape>(false);
			mesh->createShape(TOshapes[0]);
			mesh->measure();
			mesh->init();
		}
		
		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMin.x = mesh->min.x;
		gMin.y = mesh->min.y;

		//load in another mesh and make the shape(s)
		vector<tinyobj::shape_t> TOshapes2;
 		rc = tinyobj::LoadObj(TOshapes2, objMaterials, errStr, (resourceDirectory + "/bunny.obj").c_str());
		
		if (!rc) {
			cerr << errStr << endl;
		} else {
			//for now all our shapes will not have textures - change in later labs
			bunny = make_shared<Shape>(false);
			bunny->createShape(TOshapes2[0]);
			bunny->measure();
			bunny->init();
		}

		//trees
		vector<tinyobj::shape_t> TOshapes3;
 		rc = tinyobj::LoadObj(TOshapes3, objMaterials, errStr, (resourceDirectory + "/tree.obj").c_str());
		
		for(int i=0;i<TOshapes3.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				tree = make_shared<Shape>(false);
				tree->createShape(TOshapes3[i]);
				tree->measure();
				tree->init();
				trees.push_back(tree);
			}
		}

		//building
		vector<tinyobj::shape_t> TOshapes5;
 		rc = tinyobj::LoadObj(TOshapes5, objMaterials, errStr, (resourceDirectory + "/building.obj").c_str());
		
		for(int i=0;i<TOshapes5.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				building = make_shared<Shape>(false);
				building->createShape(TOshapes5[i]);
				building->measure();
				building->init();
				buildings.push_back(building);
			}
		}

		//batman
 		rc = tinyobj::LoadObj(TOshapes6, objMaterials2, errStr, (resourceDirectory + "/batman.obj").c_str(), (resourceDirectory + "/").c_str());
		
		for(int i=0;i<TOshapes6.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				batman = make_shared<Shape>(false);
				batman->createShape(TOshapes6[i]);
				batman->measure();
				batman->init();
				batmans.push_back(batman);
			}
		}

		//ground
 		rc = tinyobj::LoadObj(TOshapesGround, objMaterials, errStr, (resourceDirectory + "/cube1.obj").c_str(), (resourceDirectory + "/").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			//for now all our shapes will not have textures - change in later labs
			ground = make_shared<Shape>(true);
			ground->createShape(TOshapesGround[0]);
			ground->measure();
			ground->init();
		}

		//car
		vector<tinyobj::shape_t> TOshapes7;
 		rc = tinyobj::LoadObj(TOshapes7, objMaterials, errStr, (resourceDirectory + "/car.obj").c_str());

		for(int i=0;i<TOshapes7.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				car = make_shared<Shape>(false);
				car->createShape(TOshapes7[i]);
				car->measure();
				car->init();
				cars.push_back(car);
			}
		}

		//city
		// vector<tinyobj::shape_t> TOshapes9;
 		rc = tinyobj::LoadObj(TOshapes9, objMaterials1, errStr, (resourceDirectory + "/city.obj").c_str(), (resourceDirectory + "/").c_str());

		for(int i=0;i<TOshapes9.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				city = make_shared<Shape>(false);
				city->createShape(TOshapes9[i]);
				city->measure();
				city->init();
				cities.push_back(city);
			}
		}

		vector<tinyobj::shape_t> TOshapes15;
		rc = tinyobj::LoadObj(TOshapes15, objMaterials, errStr, (resourceDirectory + "/sphereWTex.obj").c_str());
		for(int i=0;i<TOshapes15.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				cube = make_shared<Shape>(true);
				cube->createShape(TOshapes15[0]);
				cube->measure();
				cube->init();
				cubes.push_back(cube);
			}
		}

		//batarang
		vector<tinyobj::shape_t> TOshapes10;
 		rc = tinyobj::LoadObj(TOshapes10, objMaterials, errStr, (resourceDirectory + "/batarang.obj").c_str());

		for(int i=0;i<TOshapes10.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				m4a1 = make_shared<Shape>(false);
				m4a1->createShape(TOshapes10[i]);
				m4a1->measure();
				m4a1->init();
				m4a1s.push_back(m4a1);
			}
		}
		
		//bricks
		vector<tinyobj::shape_t> TOshapes11;
 		rc = tinyobj::LoadObj(TOshapes11, objMaterials, errStr, (resourceDirectory + "/brick.obj").c_str());

		for(int i=0;i<TOshapes11.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				brick = make_shared<Shape>(false);
				brick->createShape(TOshapes11[i]);
				brick->measure();
				brick->calcNormals();
				brick->init();
				bricks.push_back(brick);
			}
		}

		//bricks1
		vector<tinyobj::shape_t> TOshapes12;
 		rc = tinyobj::LoadObj(TOshapes12, objMaterials, errStr, (resourceDirectory + "/brick.obj").c_str());

		for(int i=0;i<TOshapes12.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				brick1 = make_shared<Shape>(true);
				brick1->createShape(TOshapes12[i]);
				brick1->measure();
				brick1->calcNormals();
				brick1->init();
				bricks1.push_back(brick1);
			}
		}
	}

	// Code to load in the texture
	void initTex(const std::string& resourceDirectory)
	{
		texture = make_shared<Texture>();
		texture->setFilename(resourceDirectory + "/line.png");
		texture->init();
		texture->setUnit(0);
		texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}


	//helper function to pass material data to the GPU
	void SetMaterial(shared_ptr<Program> curS, int i) {

    	switch (i) {
    		case 0: //
    			glUniform3f(curS->getUniform("MatAmb"), 0.096, 0.046, 0.095);
    			glUniform3f(curS->getUniform("MatDif"), 0.96, 0.46, 0.95);
    			glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.2, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 120.0);
    		break;
    		case 1: // 
    			glUniform3f(curS->getUniform("MatAmb"), 0.063, 0.038, 0.1);
    			glUniform3f(curS->getUniform("MatDif"), 0.63, 0.38, 1.0);
    			glUniform3f(curS->getUniform("MatSpec"), 0.3, 0.2, 0.5);
    			glUniform1f(curS->getUniform("MatShine"), 4.0);
    		break;
    		case 2: //
    			// glUniform3f(curS->getUniform("MatAmb"), 0.004, 0.05, 0.09);
    			// glUniform3f(curS->getUniform("MatDif"), 0.04, 0.5, 0.9);
    			// glUniform3f(curS->getUniform("MatSpec"), 0.02, 0.25, 0.45);
    			// glUniform1f(curS->getUniform("MatShine"), 27.9);
				glUniform3f(curS->getUniform("MatAmb"), 0.003, 0.0, 0.003);
				glUniform3f(curS->getUniform("MatDif"), 0.03, 0.35, 0.55);
				glUniform3f(curS->getUniform("MatSpec"), 0.01, 0.125, 0.325);
				glUniform1f(curS->getUniform("MatShine"), 27.9);
    		break;
  		}
	}

	void printMaterial(tinyobj:: material_t material){
		cout << "Ambient: " << material.ambient[0] << " " << material.ambient[1] << " " << material.ambient[2] << endl;
		cout << "Diffuse: " << material.diffuse[0] << " " << material.diffuse[1] << " " << material.diffuse[2] << endl;
		cout << "Specular: " << material.specular[0] << " " << material.specular[1] << " " << material.specular[2] << endl;
		cout << "Shininess: " << material.shininess << endl;
	}

	//set material with the glsl values:
	void SetMaterialVal(shared_ptr<Program> curS, tinyobj:: material_t material) {
		glUniform3f(curS->getUniform("MatAmb"), 0.003*material.ambient[0], 0*material.ambient[1], 0.003*material.ambient[2]);
		glUniform3f(curS->getUniform("MatDif"), 0.003*material.diffuse[0], 0.35*material.diffuse[1], 0.55*material.diffuse[2]);
		glUniform3f(curS->getUniform("MatSpec"), 0.01*material.specular[0], 0.01*material.specular[1], 0.01*material.specular[2]);
		glUniform1f(curS->getUniform("MatShine"), material.shininess);
	}


	/* helper for sending top of the matrix strack to GPU */
	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   }

	/* helper function to set model trasnforms */
  	void setModel(shared_ptr<Program> curS, vec3 trans, float rotY, float rotX, float sc) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  		mat4 ctm = Trans*RotX*RotY*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	/* camera controls for animation transitions */
	void SetView(shared_ptr<Program>  shader) {
		g_lookAt = vec3(7.0, 2.2, -1);
  		glm::mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
		glm::mat4 Trans = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0));
    	Cam = Trans * Cam;
  		glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
	}

   	/* camera controls - do not change */
	void SetView2(shared_ptr<Program>  shader) {
  		glm::mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
		glm::mat4 Trans = glm::translate(glm::mat4(1.0f), glm::vec3(0,0,0));
    	Cam = Trans * Cam;
  		glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
	}

	void updateUsingCameraPath(float frametime)  {

   	  	if(goCamera) {
			if(!splinepath[0].isDone()){
				splinepath[0].update(frametime);
				g_eye = splinepath[0].getPosition();
			} else {
				g_eye = initialEye;
				g_lookAt = initialLookAt;
				goCamera = false;
			}
    	}
   	}

	void render(float frametime) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Create the matrix stacks for particles
		auto V = make_shared<MatrixStack>();

		// View for particles
		V->pushMatrix();
		V->loadIdentity();
		glm::mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
		Cam = glm::translate(Cam, vec3(0, 0, -5));
		Cam = glm::rotate(Cam, sTheta, vec3(0, 1, 0));
		V->multMatrix(Cam);

		//update the camera position
		updateUsingCameraPath(frametime);

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 1000.0f);

		// View for world
		View->pushMatrix();
			View->loadIdentity();
			View->rotate(sTheta, vec3(0, -0.5, 0));
			View->translate(vec3(gTrans - 4,gTrans2 - 3, -20 + gMove));

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		if(!goCamera){
			// glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
			SetView2(prog);
			}
			else{
				SetView(prog);
			}
		glUniform3f(prog->getUniform("lightPos"), 7.0 + gLight, 5.0, -1.5);
		glUniform1f(prog->getUniform("lightIntensity"), 1.3);
		glUniform3f(prog->getUniform("lightPos2"), 50.0, 10.0, -2);

		//for lightning effect
		if(rand()%78 == 0){
			lightCounter = 14;
		}
		if(lightCounter > 0){
			glUniform1f(prog->getUniform("lightIntensity2"), 2.3);
			lightCounter--;
		}
		else{
			glUniform1f(prog->getUniform("lightIntensity2"), 0);
		}

		//use helper function that uses glm to create some transform matrices
		setModel(prog, vec3(1.7, -1.7, 0), 0, 0, 0.5);

		//car
		for(int i=0; i<cars.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(15, -1, -5));
				Model->scale(vec3(0.35, 0.35, 0.35));
				Model->rotate((0.7), vec3(0, 1, 0));
				SetMaterial(prog, 2);
				setModel(prog, Model);
				cars[i]->draw(prog);
			Model->popMatrix();
		}

		//batman
		for(int i=0; i<batmans.size();i++) {
			Model->pushMatrix();
				Model->rotate((3.14), vec3(0, 0, 1));
				Model->rotate((3.14/2), vec3(-1, 0, 0));
				Model->rotate((3.14), vec3(0, 1, 0));
				Model->translate(vec3(7.0, 2.2, -1));
				Model->scale(vec3(0.7, 0.7, 0.7));

				int materialBatManId = TOshapes6[i].mesh.material_ids[0];
				tinyobj::material_t material = objMaterials2[materialBatManId];
				SetMaterialVal(prog, material);

				// SetMaterial(prog, 2);
				setModel(prog, Model);
				batmans[i]->draw(prog);
			Model->popMatrix();
		}

		//brick behind batman
		Model->pushMatrix();
			Model->translate(vec3(6.2, -1.5, -3));
			Model->scale(vec3(0.0065, 0.005, 0.005));
			Model->rotate(0.5*(3.14), vec3(-1, 0, 0));
			SetMaterial(prog, 2);
			setModel(prog, Model);
			bricks[0]->draw(prog);
		Model->popMatrix();

		// bricks
		if(lightCounter <= 0){
			Model->pushMatrix();
				Model->translate(vec3(7, -0.8, 5));
				Model->scale(vec3(0.005, 0.005, 0.005));
				Model->rotate(0.5*(3.14), vec3(0, 1, 0));
				SetMaterial(prog, 2);
				setModel(prog, Model);
				bricks[0]->draw(prog);
			Model->popMatrix();

			Model->pushMatrix();
				Model->translate(vec3(7.3, -0.7, 4));
				Model->scale(vec3(0.005, 0.005, 0.005));
				// Model->rotate(0.5*(3.14), vec3(0, 1, 0));
				Model->rotate(0.25*(3.14), vec3(0, 0, 1));
				SetMaterial(prog, 2);
				setModel(prog, Model);
				bricks[0]->draw(prog);
			Model->popMatrix();
		}


		//bricks
		Model->pushMatrix();
			Model->translate(vec3(15, -0.8, 5));
			Model->scale(vec3(0.005, 0.005, 0.005));
			Model->rotate(0.25*(3.14), vec3(0, 1, 0));
			SetMaterial(prog, 2);
			setModel(prog, Model);
			bricks[0]->draw(prog);
		Model->popMatrix();

		if(!switchShader){
			//bricks
			Model->pushMatrix();
				Model->translate(vec3(13, -1.5, 5));
				Model->scale(vec3(0.005, 0.005, 0.005));
				Model->rotate(0.5*(3.14), vec3(-1, 0, 0));
				SetMaterial(prog, 2);
				setModel(prog, Model);
				bricks[0]->draw(prog);
			Model->popMatrix();
		}

		//bricks
		Model->pushMatrix();
			Model->translate(vec3(16.5, -0.8, 10));
			Model->scale(vec3(0.005, 0.005, 0.005));
			Model->rotate(0.5*(3.14), vec3(0, 0, -1));
			Model->rotate(0.5*(3.14), vec3(-1, 0, 0));
			SetMaterial(prog, 2);
			setModel(prog, Model);
			bricks[0]->draw(prog);
		Model->popMatrix();


		//city
		for(int i=0; i<cities.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(-30, -1, -25));
				Model->scale(vec3(0.27, 0.27, 0.27));
				Model->rotate((3.14), vec3(0, 1, 0));

				int materialId = TOshapes9[i].mesh.material_ids[0];
				tinyobj::material_t material = objMaterials1[materialId];
				// SetMaterialVal(prog, material);
				SetMaterial(prog, 2);

				setModel(prog, Model);
				cities[i]->draw(prog);
			Model->popMatrix();
		}
		prog->unbind();

		if(switchShader){
			texProg->bind();
			glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			if(!goCamera){
				// glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
				SetView2(texProg);
			}
			else{
				SetView(texProg);
			}

			 //bricks
			Model->pushMatrix();
				Model->translate(vec3(13, -1.4, 5));
				Model->scale(vec3(0.005, 0.005, 0.005));
				Model->rotate(0.5*(3.14), vec3(-1, 0, 0));
				texture0->bind(texProg->getUniform("Texture0"));
				setModel(texProg, Model);
				bricks1[0]->draw(texProg);
			Model->popMatrix();
			texProg->unbind();
		}

		// Draw the skybox
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		if(!goCamera){
			// glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
			SetView2(texProg);
			}
			else{
				SetView(texProg);
			}
			//skybox
		for(int i=0; i<cubes.size();i++) {
			Model->pushMatrix();
				Model->loadIdentity();
				Model->translate(vec3(0, 55, 0));
				Model->scale(vec3(200, 100, 225));
				if(lightCounter > 0){
					skyBoxTexture2->bind(texProg->getUniform("Texture0"));
				}
				else{
					skyBoxTexture->bind(texProg->getUniform("Texture0"));
				}
				setModel(texProg, Model);
				cubes[i]->draw(texProg);
			Model->popMatrix();
		}

			//ground
			//disable the blending for non particles
			CHECKED_GL_CALL(glDisable(GL_BLEND));
			// Model->pushMatrix();
			// 	Model->loadIdentity();
			// 	Model->translate(vec3(0, 0, 0));
			// 	Model->scale(vec3(50, 0.1, 50));
			// 	groundTexture->bind(texProg->getUniform("Texture0"));
			// 	setModel(texProg, Model);
			// 	ground->draw(texProg);
			// Model->popMatrix();

		texProg->unbind();

		solidColorProg->bind();
		glUniformMatrix4fv(solidColorProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		if(!goCamera){
			// glUniformMatrix4fv(solidColorProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
			SetView2(solidColorProg);
			}
			else{
				SetView(solidColorProg);
			}

		GLint solidColorUni = solidColorProg->getUniform("solidColor");
		glUniform3f(solidColorUni, 0, 0, 0);

		if(lightCounter > 0){
			//batman shadowcast on lightining
			for(int i=14; i<16;i++) {
				Model->pushMatrix();
					Model->rotate((3.14), vec3(0, 0, 1));
					Model->rotate((3.14/2), vec3(-1, 0, 0));
					Model->rotate((3.14), vec3(0, 1, 0));
					Model->translate(vec3(7.4, 0.4, -0.83));
					Model->scale(vec3(0.7, 2.7, 0.1));
					Model->rotate((3.14/8), vec3(0, 0, 1));
					Model->rotate((3.14/2), vec3(-1, 0, 0));
					Model->rotate((3.14/8), vec3(0, 0, 1));
					Model->rotate((3.14), vec3(1, 0, 0));

					// SetMaterial(prog, 2);
					setModel(solidColorProg, Model);
					batmans[i]->draw(solidColorProg);
				Model->popMatrix();
			}
			Model->pushMatrix();
				Model->translate(vec3(6.2, -1.0, -5.7));
				Model->scale(vec3(0.0065, 0.001, 0.027));
				setModel(solidColorProg, Model);
				bricks[0]->draw(solidColorProg);
			Model->popMatrix();


			//car shadow on lightining
			for(int i=0; i<cars.size();i++) {
				Model->pushMatrix();
					Model->translate(vec3(16.8, -1, -1.6));
					Model->scale(vec3(0.55, 0.01, 0.65));
					Model->rotate((0.8), vec3(0, 1, 0));
					setModel(solidColorProg, Model);
					cars[i]->draw(solidColorProg);
				Model->popMatrix();
			}

			//other bricks shadow:
			Model->pushMatrix();
				Model->translate(vec3(13, -1.0, 3.3));
				Model->scale(vec3(0.005, 0.001, 0.015));
				setModel(solidColorProg, Model);
				bricks[0]->draw(solidColorProg);
			Model->popMatrix();

			// bricks
			Model->pushMatrix();
				Model->translate(vec3(7, -0.8, 5));
				Model->scale(vec3(0.005, 0.005, 0.005));
				Model->rotate(0.5*(3.14), vec3(0, 1, 0));
				setModel(solidColorProg, Model);
				bricks[0]->draw(solidColorProg);
			Model->popMatrix();

			Model->pushMatrix();
				Model->translate(vec3(7.3, -0.7, 4));
				Model->scale(vec3(0.005, 0.005, 0.005));
				// Model->rotate(0.5*(3.14), vec3(0, 1, 0));
				Model->rotate(0.25*(3.14), vec3(0, 0, 1));
				setModel(solidColorProg, Model);
				bricks[0]->draw(solidColorProg);
			Model->popMatrix();

			//other bricks
			Model->pushMatrix();
				Model->translate(vec3(16.5, -0.97, 11));
				Model->scale(vec3(0.005, 0.0001, 0.024));
				Model->rotate(0.5*(3.14), vec3(0, 0, -1));
				Model->rotate(0.5*(3.14), vec3(-1, 0, 0));
				setModel(solidColorProg, Model);
				bricks[0]->draw(solidColorProg);
			Model->popMatrix();

			//bricks
			Model->pushMatrix();
				Model->translate(vec3(14.3, -0.97, 4.5));
				Model->scale(vec3(0.01, 0.0001, 0.013));
				Model->rotate(0.25*(3.14), vec3(0, 1, 0));
				setModel(solidColorProg, Model);
				bricks[0]->draw(solidColorProg);
			Model->popMatrix();

		}

		//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(6 + rand()%4, 7 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(8 + rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(10 + rand()%4, 6 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}
		
		//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(3 + rand()%4, 7 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(-rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(4 + rand()%4, 6 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}
		
				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3( 12 + rand()%4, 11 - rand()%2, 10*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(6 + rand()%4, 10 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(8 + rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(10 + rand()%4, 6 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3( 12 + rand()%4, 7 - rand()%2, 10*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}


				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(6 + rand()%4, 14 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(rand()%4, 6 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(rand()%4, 7 - rand()%2, 10*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}


		//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(rand()%4, 13 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

		//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(2 + rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

		//batarang
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(5 + rand()%4, 12 - rand()%2, 10*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

		solidColorProg->unbind();

		// camera rotate
		thePartSystem->setCamera(V->topMatrix());

		// Draw particles
		//enable the blending for particles
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		partProg->bind();
		Model->pushMatrix();
		Model->loadIdentity();
		texture->bind(partProg->getUniform("alphaTexture"));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix())));
		
		CHECKED_GL_CALL(glUniform3f(partProg->getUniform("pColor"), 0.0, 0.5, 1.0));

		// setModel(partProg, Model);
		
		thePartSystem->drawMe(partProg);
		thePartSystem->update(false);
		thePartSystem2->drawMe(partProg);
		thePartSystem2->update(true);
		Model->popMatrix();
		partProg->unbind();

		// Pop matrix stacks.
		V->popMatrix();

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(720, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initTex(resourceDir);
	application->initGeom(resourceDir);

	auto lastTime = chrono::high_resolution_clock::now();

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{

		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		float deltaTime =
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
				.count();
		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;


		// Render scene.
		application->render(deltaTime);

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
