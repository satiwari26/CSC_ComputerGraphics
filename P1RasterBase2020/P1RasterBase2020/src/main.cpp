/* Release code for program 1 CPE 471 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include <limits>

#include "tiny_obj_loader.h"
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

int g_width, g_height;

/*
   Helper function you will want all quarter
   Given a vector of shapes which has already been read from an obj file
   resize all vertices to the range [-1, 1]
 */
void resize_obj(std::vector<tinyobj::shape_t> &shapes){
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
         if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];

         if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
         if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];

         if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
         if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
      }
   }

	//From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
      maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = 2.0 /maxExtent;
   shiftX = minX + (xExtent/ 2.0);
   scaleY = 2.0 / maxExtent;
   shiftY = minY + (yExtent / 2.0);
   scaleZ = 2.0/ maxExtent;
   shiftZ = minZ + (zExtent)/2.0;

   //Go through all verticies shift and scale them
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
         assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
         assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
         assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
      }
   }
}

/**
 * @brief
 * to keep track of the minX, minY and maxX, maxY
*/
struct triTrack{
   int minX = numeric_limits<int>::infinity();
   int maxX = -numeric_limits<int>::infinity();
   int minY = numeric_limits<int>::infinity();
   int maxY = -numeric_limits<int>::infinity();
};

/**
 * @brief
 * for screen resolution
*/
struct screenView{
   float left;
   float right;
   float bottom;
   float top;
};

struct vertexColor{
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

int worldToPixelX(float &px, screenView &view){
   float C = float(g_width - 1)/float(view.right - view.left);
   float D = (-1)*float(C*view.left);

   return(C*px + D);
}
int worldToPixelY(float &py, screenView &view){
   float E = float(g_height-1)/float(view.top - view.bottom);
   float F = (-1)*float(E*view.bottom);
   return(E*py + F);
}


int main(int argc, char **argv)
{
	if(argc < 3) {
      cout << "Usage: raster meshfile imagefile" << endl;
      return 0;
   }
	// OBJ filename
	string meshName(argv[1]);
	string imgName(argv[2]);

	//set g_width and g_height appropriately!
	g_width = 1024;
   g_height = 600;

   //create an image
	auto image = make_shared<Image>(g_width, g_height);

   //set the screen view:
   screenView viewScreen;
   if(g_width > g_height){
      viewScreen.left = float(-g_width)/float(g_height);
      viewScreen.right = float(g_width)/float(g_height);
      viewScreen.bottom = -1;
      viewScreen.top = 1;
   }
   else{
      viewScreen.left = -1;
      viewScreen.right = 1;
      viewScreen.bottom = float(-g_height)/float(g_width);
      viewScreen.top = float(g_height)/float(g_width);
   }

	// triangle buffer
	vector<unsigned int> triBuf;
	// position buffer
	vector<float> posBuf;
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes; // geometry
	vector<tinyobj::material_t> objMaterials; // material
	string errStr;
	
   bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	/* error checking on read */
	if(!rc) {
		cerr << errStr << endl;
	} else {
 		//keep this code to resize your object to be within -1 -> 1
   	resize_obj(shapes); 
		posBuf = shapes[0].mesh.positions;
		triBuf = shapes[0].mesh.indices;
	}
	cout << "Number of vertices: " << posBuf.size()/3 << endl;
	cout << "Number of triangles: " << triBuf.size()/3 << endl;

   //vertex colors
	vertexColor v1,v2,v3;
	v1.b = 173;
	v1.g = 216;
	v1.r = 230;

	v2.b = 173;
	v2.g = 216;
	v2.r = 230;

	v3.b = 173;
	v3.g = 216;
	v3.r = 230;

   vector<vector<float>> Zbuff(g_height,vector<float>(g_width,-numeric_limits<float>::infinity()));

	//TODO add code to iterate through each triangle and rasterize it 
   for (size_t i = 0; i < shapes.size(); i++) {
      //vertex1
      int V1x;
      int V1y;
      float V1z;
      //vertex2
      int V2x;
      int V2y;
      float V2z;
      //vertex3
      int V3x;
      int V3y;
      float V3z;

      cout<<shapes[i].mesh.indices.size()<<endl;
      cout<<shapes[i].mesh.positions.size()<<endl;

      for (size_t v = 0; v < shapes[i].mesh.indices.size(); v +=3) {
         V1x = worldToPixelX(shapes[i].mesh.positions[3*shapes[i].mesh.indices[v]],viewScreen);
         V1y = worldToPixelY(shapes[i].mesh.positions[3*shapes[i].mesh.indices[v]+1],viewScreen);
         V1z = shapes[i].mesh.positions[3*shapes[i].mesh.indices[v]+2];

         V2x = worldToPixelX(shapes[i].mesh.positions[3*shapes[i].mesh.indices[v+1]],viewScreen);
         V2y = worldToPixelY(shapes[i].mesh.positions[3*shapes[i].mesh.indices[v+1]+1],viewScreen);
         V2z = shapes[i].mesh.positions[3*shapes[i].mesh.indices[v+1]+2];

         V3x = worldToPixelX(shapes[i].mesh.positions[3*shapes[i].mesh.indices[v+2]],viewScreen);
         V3y = worldToPixelY(shapes[i].mesh.positions[3*shapes[i].mesh.indices[v+2]+1],viewScreen);
         V3z = shapes[i].mesh.positions[3*shapes[i].mesh.indices[v+2]+2];

         //find the min and max x and y coordinate val:
         triTrack track;

         //find minX
         track.minX = min(V1x,V2x);
         track.minX = min(track.minX,V3x);

         //find minY
         track.minY = min(V1y,V2y);
         track.minY = min(track.minY,V3y);

         //find maxX
         track.maxX = max(V1x,V2x);
         track.maxX = max(track.maxX,V3x);

         //find maxY
         track.maxY = max(V1y,V2y);
         track.maxY = max(track.maxY,V3y);

         //calculate the alpha, beta, gamma values
         for(int y = track.minY; y <= track.maxY; ++y) {
            for(int x = track.minX; x <= track.maxX; ++x) {

               //for each point we need to check if it lies inside the traingle or outside:
               float gamma = float((V2x-V1x)*(y-V1y) - (x-V1x)*(V2y-V1y))/float((V2x-V1x)*(V3y-V1y) - (V3x-V1x)*(V2y - V1y));
               float beta = float((V1x-V3x)*(y-V3y) - (x-V3x)*(V1y-V3y))/float((V2x-V1x)*(V3y-V1y) - (V3x-V1x)*(V2y - V1y));
               float alpha = (1.0 - beta - gamma);

               if(alpha >= 0.15 && alpha <= 1.001 && gamma >= 0.15 && gamma <= 1.001 && beta >= 0.15 && beta < 1.001 ) {

                  float currentZ = alpha*float(V1z) + beta*float(V2z) + gamma*float(V3z);

                  if(Zbuff[y][x] < currentZ){
                     unsigned char r = 255*((currentZ + 1)/2);
                     image->setPixel(x , y, 0.75*r, 0, 0);
                     Zbuff[y][x] = currentZ;
                  }
               }
            }
         }


      }

   }
	
	//write out the image
   image->writeToFile(imgName);

	return 0;
}
