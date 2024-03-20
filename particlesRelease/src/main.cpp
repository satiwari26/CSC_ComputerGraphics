/**
 * Base code for particle systems lab
 */

#include <iostream>
#include <algorithm>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "WindowManager.h"
#include "particleSys.h"

 // value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;


class Application : public EventCallbacks
{

public:

	WindowManager *windowManager = nullptr;

	// Our shader program for particles
	std::shared_ptr<Program> partProg;

	// Our shader program for meshes
	std::shared_ptr<Program> meshProg;

	//the partricle system
	particleSys *thePartSystem;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> sphere;

	// OpenGL handle to texture data used in particle
	shared_ptr<Texture> texture;

	bool keyToggles[256] = { false };

	//some particle variables
	float t = 0.0f; //reset in init
	float h = 0.01f;
	float movement = 0.0f;

	glm::vec3 g = glm::vec3(0.0f, -0.01f, 0.0f);
	float camRot;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		keyToggles[key] = ! keyToggles[key];

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			camRot -= 0.314f;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			camRot += 0.314f;
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			movement += 0.314f;
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			thePartSystem->reSet();
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		CHECKED_GL_CALL(glViewport(0, 0, width, height));
	}

	//code to set up the two shaders - a diffuse shader and texture mapping
	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		// Set background color.
		CHECKED_GL_CALL(glClearColor(0,0,0, 1.0f));

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

				// Initialize the GLSL program.
		meshProg = make_shared<Program>();
		meshProg->setVerbose(true);
		meshProg->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (! meshProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		meshProg->addUniform("P");
		meshProg->addUniform("M");
		meshProg->addUniform("V");
		meshProg->addAttribute("vertPos");
		meshProg->addAttribute("vertNor");

		thePartSystem = new particleSys(vec3(0, 2.5, 0));
		thePartSystem->gpuSetup();
	}

	void initGeom(const std::string& resourceDirectory){

		//EXAMPLE new set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/SmoothSphere.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			sphere = make_shared<Shape>();
			sphere->createShape(TOshapes[0]);
			sphere->measure();
			sphere->init();
		}
		//read out information stored in the shape about its size - something like this...

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

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = width / (float) height;

		// Create the matrix stacks
		auto P = make_shared<MatrixStack>();
		auto V = make_shared<MatrixStack>();
		auto M = make_shared<MatrixStack>();

		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 100.0f);

		// View is identity - for now
		V->pushMatrix();
		V->translate(vec3(0, 0, -5 + movement));
		V->rotate(camRot, vec3(0, 1, 0));

		M->pushMatrix();
		M->loadIdentity();

		// camera rotate
		thePartSystem->setCamera(V->topMatrix());

		meshProg->bind();
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix())));

		// draw mesh 
		M->pushMatrix();
			M->loadIdentity();
			//"global" translate
			M->pushMatrix();
				M->scale(vec3(0.5, 0.5, 0.5));
				glUniformMatrix4fv(meshProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
				sphere->draw(meshProg);
			M->popMatrix();
		M->popMatrix();

		meshProg->unbind();

		// Draw
		partProg->bind();
		texture->bind(partProg->getUniform("alphaTexture"));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix())));
		
		CHECKED_GL_CALL(glUniform3f(partProg->getUniform("pColor"), 0.9, 0.7, 0.7));
		
		thePartSystem->drawMe(partProg);
		thePartSystem->update();

		partProg->unbind();

		// Pop matrix stacks.
		V->popMatrix();
		M->popMatrix();
		P->popMatrix();
	}

};

int main(int argc, char **argv)
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
	windowManager->init(512, 512);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initTex(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
