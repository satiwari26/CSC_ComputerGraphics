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

	// Our shader program
	std::shared_ptr<Program> solidColorProg;

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

	//batman
	shared_ptr<Shape> batman;
	vector<shared_ptr<Shape>> batmans;

	//grass
	shared_ptr<Shape> grass;
	vector<shared_ptr<Shape>> grasses;

	//Mountain
	shared_ptr<Shape> mountain;
	vector<shared_ptr<Shape>> mountains;

	//city
	shared_ptr<Shape> city;
	vector<shared_ptr<Shape>> cities;

	//M4A1
	shared_ptr<Shape> m4a1;
	vector<shared_ptr<Shape>> m4a1s;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	//animation data
	float sTheta = 0;
	float gTrans = -6;
	float gTrans2 = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			gTrans += 0.2;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			gTrans -= 0.2;
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			gTrans2 -= 0.2;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			gTrans2 += 0.2;
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			sTheta -= 0.2;
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
		vector<tinyobj::shape_t> TOshapes6;
 		rc = tinyobj::LoadObj(TOshapes6, objMaterials, errStr, (resourceDirectory + "/batman.obj").c_str());
		
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

		//mountain
		vector<tinyobj::shape_t> TOshapes8;
 		rc = tinyobj::LoadObj(TOshapes8, objMaterials, errStr, (resourceDirectory + "/mountain.obj").c_str());

		for(int i=0;i<TOshapes8.size();i++){
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for now all our shapes will not have textures - change in later labs
				mountain = make_shared<Shape>(false);
				mountain->createShape(TOshapes8[i]);
				mountain->measure();
				mountain->init();
				mountains.push_back(mountain);
			}
		}

		//city
		vector<tinyobj::shape_t> TOshapes9;
 		rc = tinyobj::LoadObj(TOshapes9, objMaterials, errStr, (resourceDirectory + "/city.obj").c_str());

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

		//M4A1
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

		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMin.x = mesh->min.x;
		gMin.y = mesh->min.y;
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
			View->rotate(sTheta, vec3(0, -0.5, 0));
			View->translate(vec3(gTrans, gTrans2, -20));

		// Draw a solid colored sphere
		// solidColorProg->bind();
		// //send the projetion and view for solid shader
		// glUniformMatrix4fv(solidColorProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		// glUniformMatrix4fv(solidColorProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		// //send in the color to use
		// glUniform3f(solidColorProg->getUniform("solidColor"), 0.1, 0.2, 0.5);

		// //use helper function that uses glm to create some transform matrices
		// setModel(prog, vec3(-1.7, -1.7, 0), 0, 0, 0.5);
		// mesh->draw(prog);

		// solidColorProg->unbind();

		// Draw base Hierarchical person
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		//use helper function that uses glm to create some transform matrices
		setModel(prog, vec3(1.7, -1.7, 0), 0, 0, 0.5);

		// //Trees
		// for(int i=0; i<trees.size();i++) {
		// 	Model->pushMatrix();
		// 		Model->translate(vec3(-18, -5, 8));
		// 		Model->scale(vec3(2, 2, 2));
		// 		Model->rotate((0.3), vec3(0, 1, 0));
		// 		setModel(prog, Model);
		// 		trees[i]->draw(prog);
		// 	Model->popMatrix();
		// }

		// //building
		// for(int i=0; i<buildings.size();i++) {
		// 	Model->pushMatrix();
		// 		Model->translate(vec3(2, -2, 2));
		// 		Model->scale(vec3(2, 2, 2));
		// 		Model->rotate((0.3), vec3(0, 1, 0));
		// 		setModel(prog, Model);
		// 		buildings[i]->draw(prog);
		// 	Model->popMatrix();
		// }

		//car
		for(int i=0; i<cars.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(15, -1, -5));
				Model->scale(vec3(0.35, 0.35, 0.35));
				Model->rotate((0.7), vec3(0, 1, 0));
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
				setModel(prog, Model);
				batmans[i]->draw(prog);
			Model->popMatrix();
		}

		//mountain
		for(int i=0; i<mountains.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(-30, 5, 30));
				Model->scale(vec3(0.5, 0.5, 0.5));
				Model->rotate((3.14), vec3(0, 1, 0));
				setModel(prog, Model);
				mountains[i]->draw(prog);
			Model->popMatrix();
		}

		//city
		for(int i=0; i<cities.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(-30, -1, -25));
				Model->scale(vec3(0.27, 0.27, 0.27));
				Model->rotate((3.14), vec3(0, 1, 0));
				setModel(prog, Model);
				cities[i]->draw(prog);
			Model->popMatrix();
		}


		//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(6 + rand()%4, 2 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(8 + rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(10 + rand()%4, 6 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}


		
				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(3 + rand()%4, 2 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(1 + rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(4 + rand()%4, 6 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}





				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3( 12 + rand()%4, 2 - rand()%2, 10*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(6 + rand()%4, 2 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(8 + rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(10 + rand()%4, 6 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3( 12 + rand()%4, 2 - rand()%2, 10*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}


				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(6 + rand()%4, 2 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(rand()%4, 6 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(rand()%4, 2 - rand()%2, 10*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}


						//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(rand()%4, 13 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(2 + rand()%4, 9 - rand()%2, 20*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}

				//M4A1
		for(int i=0; i<m4a1s.size();i++) {
			Model->pushMatrix();
				Model->translate(vec3(5 + rand()%4, 12 - rand()%2, 10*sin(glfwGetTime())));
				Model->scale(vec3(0.005,0.005,0.005));
				Model->rotate((3.14), vec3(0, 1, 0.5 - sin(rand()%2)));
				setModel(prog, Model);
				m4a1s[i]->draw(prog);
			Model->popMatrix();
		}



		prog->unbind();

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
