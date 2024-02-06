/* Lab 4 base code - transforms using local matrix functions 
   to be written by students - <2019 merge with tiny loader changes to support multiple objects>
	CPE 471 Cal Poly Z. Wood + S. Sueda
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

class Application : public EventCallbacks
{

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


	void printMat(float *A, const char *name = 0)
	{
   // OpenGL uses col-major ordering:
   // [ 0  4  8 12]
   // [ 1  5  9 13]
   // [ 2  6 10 14]
   // [ 3  7 11 15]
   // The (i,j)th element is A[i+4*j].
		if(name) {
			printf("%s=[\n", name);
		}
		for(int i = 0; i < 4; ++i) {
			for(int j = 0; j < 4; ++j) {
				printf("%- 5.2f ", A[i+4*j]);
			}
			printf("\n");
		}
		if(name) {
			printf("];");
		}
		printf("\n");
	}

	void createIdentityMat(float *M)
	{
	//set all values to zero
		for(int i = 0; i < 4; ++i) {
			for(int j = 0; j < 4; ++j) {
				M[i*4+j] = 0;
			}
		}
	//overwrite diagonal with 1s
		M[0] = M[5] = M[10] = M[15] = 1;
	}

	void createTranslateMat(float *T, float x, float y, float z)
	{
		createIdentityMat(T);
		T[12] = x;
		T[13] = y;
		T[14] = z;
	}


	void createScaleMat(float *S, float x, float y, float z)
	{
		S[0] = x;
		S[5] = y;
		S[10] = z;
		S[15] = 1;
	}

	void createRotateMatX(float *R, float radians)
	{ 
		createIdentityMat(R);
		float rad = radians; // Convert to radians
		float cosAngle = cos(rad);
		float sinAngle = sin(rad);

		// Set the rotation components
		R[5] = cosAngle;
		R[6] = -sinAngle;
		R[9] = sinAngle;
		R[10] = cosAngle;
	}

	void createRotateMatY(float *R, float radians)
	{
		createIdentityMat(R);
		float rad = radians; // Convert to radians
		float cosAngle = cos(rad);
		float sinAngle = sin(rad);

		// Set the rotation components
		R[0] = cosAngle;
		R[2] = -sinAngle;
		R[8] = sinAngle;
		R[10] = cosAngle;

	}

	void createRotateMatZ(float *R, float radians)
	{
		createIdentityMat(R);
		float rad = radians; // Convert to radians
		float cosAngle = cos(rad);
		float sinAngle = sin(rad);

		// Set the rotation components
		R[0] = cosAngle;
		R[1] = -sinAngle;
		R[4] = sinAngle;
		R[5] = cosAngle;
	}

	void multMat(float *C, const float *A, const float *B)
    {
        float c = 0;
        for(int k = 0; k < 4; ++k) {
      // Process kth column of C
            for(int i = 0; i < 4; ++i) {
         // Process ith row of C.
         // The (i,k)th element of C is the dot product
         // of the ith row of A and kth col of B.
                c = 0;
         //vector dot
                for(int j = 0; j < 4; ++j) {
					c += A[i + j*4] * B[j + k*4];
                }
				C[i + k*4] = c;
            }
        }

		// for(int i = 0; i < 4; ++i) {
		// 	for(int j = 0; j < 4; ++j) {
		// 		// C[j*4 + i] = 0;
		// 		c = 0;
		// 		for(int k = 0; k < 4; ++k) {
		// 			c += A[k*4 + i] * B[j*4 + k];
		// 		}
		// 		C[j*4 + i] += c;
		// 	}
   		// }
    }

	void createPerspectiveMat(float *m, float fovy, float aspect, float zNear, float zFar)
	{
   // http://www-01.ibm.com/support/knowledgecenter/ssw_aix_61/com.ibm.aix.opengl/doc/openglrf/gluPerspective.htm%23b5c8872587rree
		float f = 1.0f/glm::tan(0.5f*fovy);
		m[ 0] = f/aspect;
		m[ 1] = 0.0f;
		m[ 2] = 0.0f;
		m[ 3] = 0.0f;
		m[ 4] = 0;
		m[ 5] = f;
		m[ 6] = 0.0f;
		m[ 7] = 0.0f;
		m[ 8] = 0.0f;
		m[ 9] = 0.0f;
		m[10] = (zFar + zNear)/(zNear - zFar);
		m[11] = -1.0f;
		m[12] = 0.0f;
		m[13] = 0.0f;
		m[14] = 2.0f*zFar*zNear/(zNear - zFar);
		m[15] = 0.0f;
	}


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
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
		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh = make_shared<Shape>();
			mesh->createShape(TOshapes[0]);
			mesh->measure();
			mesh->init();
		}
		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMin.x = mesh->min.x;
		gMin.y = mesh->min.y;
	}

	void render()
	{
		//local modelview matrix use this for lab 4
		float M[16] = {0};
		float V[16] = {0};
		float P[16] = {0};

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the local matrices for lab 4
		float aspect = width/(float)height;
		createPerspectiveMat(P, 70.0f, aspect, 0.1, 100.0f);	
		createTranslateMat(V, -5, 0, -6);

		//rotate the view model
		float rotateView[16] = {0};
		float rotateViewResult[16] = {0};
		createRotateMatY(rotateView, -0.5);
		multMat(rotateViewResult, rotateView, V);


		float cubeTrans[16] = {0};
		float cubeScale[16] = {0};
		createScaleMat(cubeScale, 1, 5, 1);
		createTranslateMat(cubeTrans, -2, 0, 0);
		multMat(M, cubeTrans, cubeScale);		

		// Draw mesh using GLSL.
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, rotateViewResult);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, M);
		mesh->draw(prog);
		prog->unbind();

		float cubeTrans1[16] = {0};
		float cubeScale1[16] = {0};
		createScaleMat(cubeScale1, 1, 5, 1);
		createTranslateMat(cubeTrans1, 0, 0, 0);
		multMat(M, cubeTrans1, cubeScale1);

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, rotateViewResult);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, M);
		mesh->draw(prog);
		prog->unbind();

		float cubeTrans2[16] = {0};
		float cubeScale2[16] = {0};
		createScaleMat(cubeScale2, 1, 5, 1);
		createTranslateMat(cubeTrans2, 2, 0, 0);
		multMat(M, cubeTrans2, cubeScale2);

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, rotateViewResult);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, M);
		mesh->draw(prog);
		prog->unbind();

		
		float cubeTrans3[16] = {0};
		float cubeScale3[16] = {0};
		float rotateMat[16] = {0};
		float result[16] = {0};
		createScaleMat(cubeScale3, 4, 1, 1);
		createRotateMatZ(rotateMat, 0.5);
		multMat(result, rotateMat, cubeScale3);
		createTranslateMat(cubeTrans3, -1.00, 0, 0);
		
		multMat(M, cubeTrans3, result);

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, P);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, rotateViewResult);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, M);
		mesh->draw(prog);
		prog->unbind();

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
