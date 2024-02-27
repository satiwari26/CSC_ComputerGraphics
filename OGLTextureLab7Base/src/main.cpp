/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program - use this one for Blinn-Phong
	std::shared_ptr<Program> prog;

	//Our shader program for textures
	std::shared_ptr<Program> texProg;

	//our geometry
	shared_ptr<Shape> sphere;

	shared_ptr<Shape> theBunny;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	//the image to use as a texture (ground)
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;
	shared_ptr<Texture> texture2;	

	//global data (larger program should be encapsulated)
	vec3 gMin;
	float gRot = 0;
	float gCamH = 0;
	//animation data
	float lightTrans = 0;
	float gTrans = -3;
	float sTheta = 0;
	float eTheta = 0;
	float hTheta = 0;

	bool flipNormals = false;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		//update global camera rotate
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			gRot -= 0.2;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			gRot += 0.2;
		}
		//update camera height
		if (key == GLFW_KEY_S && action == GLFW_PRESS){
			gCamH  += 0.25;
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS){
			gCamH  -= 0.25;
		}

		if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			lightTrans += 0.5;
		}
		if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT)){
			lightTrans -= 0.5;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		if (key == GLFW_KEY_F && action == GLFW_PRESS) {
			flipNormals = !flipNormals;
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
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program that we will use for local shading
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
		prog->addUniform("lightPos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("flip");
		texProg->addUniform("Texture0");
		texProg->addUniform("MatShine");
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

  		texture1 = make_shared<Texture>();
  		texture1->setFilename(resourceDirectory + "/crate.jpg");
  		texture1->init();
  		texture1->setUnit(1);
  		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  		texture2 = make_shared<Texture>();
  		texture2->setFilename(resourceDirectory + "/world.jpg");
  		texture2->init();
  		texture2->setUnit(2);
  		texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

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
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/sphereWTex.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			sphere = make_shared<Shape>();
			sphere->createShape(TOshapes[0]);
			sphere->measure();
			sphere->init();
		}
		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMin.x = sphere->min.x;
		gMin.y = sphere->min.y;

		// Initialize bunny mesh.
		vector<tinyobj::shape_t> TOshapesB;
 		vector<tinyobj::material_t> objMaterialsB;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesB, objMaterialsB, errStr, (resourceDirectory + "/dog.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {	
			theBunny = make_shared<Shape>();
			theBunny->createShape(TOshapesB[0]);
			theBunny->measure();
			theBunny->init();
		}

		//code to load in the ground plane (CPU defined data passed to GPU)
		initGround();
	}

	//directly pass quad for the ground to the GPU
	void initGround() {

		float g_groundSize = 20;
		float g_groundY = -0.25;

  		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY, -g_groundSize
		};

		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		static GLfloat GrndTex[] = {
      		0, 0, // back
      		0, 1,
      		1, 1,
      		1, 0 };

      	unsigned short idx[] = {0, 1, 2, 0, 2, 3};

		//generate the ground VAO
      	glGenVertexArrays(1, &GroundVertexArrayID);
      	glBindVertexArray(GroundVertexArrayID);

      	g_GiboLen = 6;
      	glGenBuffers(1, &GrndBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndNorBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndTexBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

      	glGenBuffers(1, &GIndxBuffObj);
     	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
      }

      //code to draw the ground plane
     void drawGround(shared_ptr<Program> curS) {
     	curS->bind();
     	glBindVertexArray(GroundVertexArrayID);
     	texture0->bind(curS->getUniform("Texture0"));
		//draw the ground plane 
  		SetModel(vec3(0, -1, 0), 0, 0, 1, curS);
  		glEnableVertexAttribArray(0);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
  		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(1);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
  		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(2);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
  		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

   		// draw!
  		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

  		glDisableVertexAttribArray(0);
  		glDisableVertexAttribArray(1);
  		glDisableVertexAttribArray(2);
  		curS->unbind();
     }

     //helper function to pass material data to the GPU
	void SetMaterial(shared_ptr<Program> curS, int i) {

    	switch (i) {
    		case 0: //shiny blue plastic
    			glUniform3f(curS->getUniform("MatAmb"), 0.096, 0.046, 0.095);
    			glUniform3f(curS->getUniform("MatDif"), 0.96, 0.46, 0.95);
    			glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.23, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 120.0);
    		break;
    		case 1: // flat grey
    			glUniform3f(curS->getUniform("MatAmb"), 0.063, 0.038, 0.1);
    			glUniform3f(curS->getUniform("MatDif"), 0.63, 0.38, 1.0);
    			glUniform3f(curS->getUniform("MatSpec"), 0.3, 0.2, 0.5);
    			glUniform1f(curS->getUniform("MatShine"), 4.0);
    		break;
    		case 2: //brass
    			glUniform3f(curS->getUniform("MatAmb"), 0.004, 0.05, 0.09);
    			glUniform3f(curS->getUniform("MatDif"), 0.04, 0.5, 0.9);
    			glUniform3f(curS->getUniform("MatSpec"), 0.02, 0.25, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 27.9);
    		break;
  		}
	}

	/* helper function to set model trasnforms */
  	void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  		mat4 ctm = Trans*RotX*RotY*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   	}

   	/* code to draw waving hierarchical model */
   	void drawHierModel(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog) {
   		// draw hierarchical mesh 
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(gTrans, 0, -6));
			/* draw top sphere - aka head */
			Model->pushMatrix();
				Model->translate(vec3(0, 1.4, 0));
				Model->scale(vec3(0.5, 0.5, 0.5));
				setModel(prog, Model);
				sphere->draw(prog);
			Model->popMatrix();
			//draw the torso with these transforms
			Model->pushMatrix();
			  Model->scale(vec3(1.15, 1.35, 1.0));
			  setModel(prog, Model);
			  sphere->draw(prog);
			Model->popMatrix();
			// draw the upper 'arm' - relative 
			Model->pushMatrix();
			  //place at shoulder
			  Model->translate(vec3(0.7, 0.8, 0));
			  //rotate shoulder joint
			  Model->rotate(sTheta, vec3(0, 0, 1));
			  //move to shoulder joint
			  Model->translate(vec3(0.7, 0, 0));
			  //lower arm
			  Model->pushMatrix();
			    Model->translate(vec3(0.7, 0.0, 0));
			    Model->rotate(eTheta, vec3(0, 0, 1));
			    Model->translate(vec3(0.4, 0, 0));
				 //hand
			    Model->pushMatrix();
			      Model->translate(vec3(0.4, 0.0, 0));
			      Model->rotate(hTheta, vec3(0, 0, 1));
			      Model->translate(vec3(0.2, 0, 0));
			      Model->scale(vec3(0.4, 0.25, 0.25));
			      setModel(prog, Model);
			      sphere->draw(prog);
			    Model->popMatrix();
				 //fore arm scale
			    Model->scale(vec3(0.6, 0.25, 0.25));
			    setModel(prog, Model);
			    sphere->draw(prog);
			  Model->popMatrix();
			  //upper arm scale
			  Model->scale(vec3(0.8, 0.25, 0.25));
			  setModel(prog, Model);
			  sphere->draw(prog);
			Model->popMatrix();
			//left arm
			Model->pushMatrix();
			  //place at shoulder
			  Model->translate(vec3(-0.7, 0.8, 0));
			  //rotate shoulder joint
			  Model->rotate(0.35, vec3(0, 0, 1));
			  //move to shoulder joint
			  Model->translate(vec3(-0.7, 0, 0));
			  //lower arm
			  Model->pushMatrix();
			    Model->translate(vec3(-0.7, 0.0, 0));
			    Model->rotate(2.0, vec3(0, 0, 1));
			    Model->translate(vec3(-0.4, 0, 0));
				 //hand
			    Model->pushMatrix();
			      Model->translate(vec3(-0.4, 0.0, 0));
			      Model->rotate(-0.3, vec3(0, 0, 1));
			      Model->translate(vec3(-0.2, 0, 0));
			      Model->scale(vec3(0.4, 0.25, 0.25));
			      setModel(prog, Model);
			      sphere->draw(prog);
			    Model->popMatrix();
				 //arm scale
			    Model->scale(vec3(0.6, 0.25, 0.25));
			    setModel(prog, Model);
			    sphere->draw(prog);
			  Model->popMatrix();
			  //non-uniform scale
			  Model->scale(vec3(0.8, 0.25, 0.25));
			  setModel(prog, Model);
			  sphere->draw(prog);
		Model->popMatrix();
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
		//camera up and down
		View->translate(vec3(0, gCamH, 0));
		//global rotate (the whole scene )
		View->rotate(gRot, vec3(0, 1, 0));

		// Draw the scene
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(texProg->getUniform("lightPos"), 2.0+lightTrans, 1.0, 2.9);
		glUniform1f(texProg->getUniform("MatShine"), 27.9);
		glUniform1i(texProg->getUniform("flip"), 0);
		texture1->bind(texProg->getUniform("Texture0"));
		// draw the array of bunnies
		Model->pushMatrix();

		float dScale = 1.0/(theBunny->max.x-theBunny->min.x);
		float sp = 3.0;
		float off = -3.5;
		  for (int i =0; i < 3; i++) {
		  	for (int j=0; j < 3; j++) {
			  Model->pushMatrix();
				Model->translate(vec3(off+sp*i, -0.5, off+sp*j));
				Model->scale(vec3(dScale));
				//SetMaterial(texProg, (i+j)%3);
				glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
				theBunny->draw(texProg);
			  Model->popMatrix();
			}
		  }
		Model->popMatrix();

		//draw the waving HM
		//SetMaterial(texProg, 1);
		texture2->bind(texProg->getUniform("Texture0"));
		drawHierModel(Model, texProg);

		//draw big background sphere
		if(flipNormals) {
			glUniform1i(texProg->getUniform("flip"), 1);
		} else {
			glUniform1i(texProg->getUniform("flip"), 0);
		}
		texture0->bind(texProg->getUniform("Texture0"));
		Model->pushMatrix();
			Model->loadIdentity();
			Model->scale(vec3(8.0));
			setModel(texProg, Model);
			sphere->draw(texProg);
		Model->popMatrix();

		texProg->unbind();

		//switch shaders to the texture mapping shader and draw the ground
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		glUniform3f(texProg->getUniform("lightPos"), 2.0+lightTrans, 2.0, 2.9);
		glUniform1f(texProg->getUniform("MatShine"), 27.9);	
		glUniform1i(texProg->getUniform("flip"), 1);
		
		drawGround(texProg);

		texProg->unbind();
		
		//animation update example
		sTheta = sin(glfwGetTime());
		eTheta = std::max(0.0f, (float)sin(glfwGetTime()));
		hTheta = std::max(0.0f, (float)cos(glfwGetTime()));

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
