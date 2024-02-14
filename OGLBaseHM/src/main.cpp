/*
 * Hierarchical modeling base code (could also be used for Program 2)
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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

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

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> mesh;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	//animation data
	float sTheta = 0;
	float gTrans = 0;
	float prev_thetha = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			gTrans -= 0.2;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			gTrans += 0.2;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
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

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
	}

	void initGeom(const std::string& resourceDirectory)
	{

		//EXAMPLE set up to read one shape from one obj file - convert to read several
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
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   }

	void render() {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		// View is global translation along negative z for now
		View->pushMatrix();
			View->loadIdentity();
			View->translate(vec3(0, 0, -5));

		// Draw a stack of cubes with indiviudal transforms
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		// draw mesh 
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(gTrans, 0, 0));
			/* draw top cube - aka head */
			Model->pushMatrix();
				Model->translate(vec3(0, 1.4, 0));
				Model->scale(vec3(0.5, 0.5, 0.5));
				setModel(prog, Model);
				mesh->draw(prog);
			Model->popMatrix();
			//draw the torso with these transforms
			Model->pushMatrix();
			  Model->scale(vec3(1.25, 1.35, 1.25));
			  setModel(prog, Model);
			  mesh->draw(prog);
			Model->popMatrix();
			// draw the upper 'arm' - relative 
			//note you must change this to include 3 components!
			Model->pushMatrix();
			  //place at shoulder
			  Model->translate(vec3(0.8, 0.8, 0));
			  //rotate shoulder joint
			  Model->rotate(sTheta, vec3(0, 0, 1));
			  //move to shoulder joint
			  Model->translate(vec3(0.8, 0, 0));
	
			    //now draw lower arm - this is INCOMPLETE and you will add a 3rd component
			  	//right now this is in the SAME place as the upper arm
			  	Model->pushMatrix();
				  Model->translate(vec3(0.6, 0, 0));
				  if(prev_thetha > 0){
					Model->rotate(sTheta, vec3(0, 0, 0.5));
				  }
				  Model->translate(vec3(0.4, 0, 0));

					//upper arm
					Model->pushMatrix();
					Model->translate(vec3(0.3, 0, 0));
					if(prev_thetha > 0){
						Model->rotate(sTheta, vec3(0, 0, 0.2));
					}
					Model->translate(vec3(0.34, 0, 0));
					Model->scale(vec3(0.3, 0.3, 0.25));
					setModel(prog, Model);
					mesh->draw(prog);
					Model->popMatrix();

			      Model->scale(vec3(0.55, 0.3, 0.25));
			  	  setModel(prog, Model);
			  	  mesh->draw(prog);
			  	Model->popMatrix();

				prev_thetha = sTheta;

			  //Do final scale ONLY to upper arm then draw
			  //non-uniform scale
			  Model->scale(vec3(0.8, 0.25, 0.25));
			  setModel(prog, Model);
			  mesh->draw(prog);
			Model->popMatrix();

			//drawing the left arm:
			Model->pushMatrix();
			//place at shoulder
			Model->translate(vec3(-0.8, 0.8, 0)); // mirror the x-coordinate
			//rotate shoulder joint
			Model->rotate(sTheta, vec3(0, 0, 1));
			//move to shoulder joint
			Model->translate(vec3(-0.8, 0, 0)); // mirror the x-coordinate

			//now draw lower arm - this is INCOMPLETE and you will add a 3rd component
			//right now this is in the SAME place as the upper arm
			Model->pushMatrix();
			Model->translate(vec3(-0.6, 0, 0)); // mirror the x-coordinate
			if(prev_thetha > 0){
				Model->rotate(sTheta, vec3(0, 0, 0.5));
			}
			Model->translate(vec3(-0.4, 0, 0)); // mirror the x-coordinate

				//upper arm
				Model->pushMatrix();
				Model->translate(vec3(-0.3, 0, 0)); // mirror the x-coordinate
				if(prev_thetha > 0){
					Model->rotate(sTheta, vec3(0, 0, 0.2));
				}
				Model->translate(vec3(-0.34, 0, 0)); // mirror the x-coordinate
				Model->scale(vec3(0.3, 0.3, 0.25));
				setModel(prog, Model);
				mesh->draw(prog);
				Model->popMatrix();

			Model->scale(vec3(0.55, 0.3, 0.25));
			setModel(prog, Model);
			mesh->draw(prog);
			Model->popMatrix();

			prev_thetha = sTheta;

			//Do final scale ONLY to upper arm then draw
			//non-uniform scale
			Model->scale(vec3(0.8, 0.25, 0.25));
			setModel(prog, Model);
			mesh->draw(prog);
			Model->popMatrix();


		
		Model->popMatrix();

		prog->unbind();

		//animation update example
		sTheta = sin(glfwGetTime());

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
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
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
