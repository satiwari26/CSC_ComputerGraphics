#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "Image.h"
#include <limits.h>

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

struct vertexColor{
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

int main(int argc, char **argv)
{
	if(argc < 9) {
		cout << "Usage: Lab2 <out_image_name>.png width height" << endl;
		return 0;
	}
	// Output filename
	string filename(argv[1]);
	// Width of image
	int width = atoi(argv[2]);
	// Height of image
	int height = atoi(argv[3]);
	//vertex1 input
	int V1x = atoi(argv[4]);
	int V1y = atoi(argv[5]);
	//vertex2 input
	int V2x = atoi(argv[6]);
	int V2y = atoi(argv[7]);
	//vertex3 input
	int V3x = atoi(argv[8]);
	int V3y = atoi(argv[9]);
	// Create the image. We're using a `shared_ptr`, a C++11 feature.
	auto image = make_shared<Image>(width, height);

	//find the min and max x and y coordinate val:
	int minX = INT_MAX;
	int maxX = 0;
	int minY = INT_MAX;
	int maxY = 0;

	//find minX
	minX = min(V1x,V2x);
	minX = min(minX,V3x);

	//find minY
	minY = min(V1y,V2y);
	minY = min(minY,V3y);

	//find maxX
	maxX = max(V1x,V2x);
	maxX = max(maxX,V3x);

	//find maxY
	maxY = max(V1y,V2y);
	maxY = max(maxY,V3y);

	//vertex colors
	vertexColor v1,v2,v3;
	v1.b = 255;
	v1.g = 35;
	v1.r = 23;

	v2.b = 32;
	v2.g = 255;
	v2.r = 40;

	v3.b = 78;
	v3.g = 21;
	v3.r = 255;

	//set the vertex points
	image->setPixel(V1x, V1y, 255,255,255);
	image->setPixel(V2x, V2y, 255,255,255);
	image->setPixel(V3x, V3y, 255,255,255);

	// Draw a rectangle
	for(int y = minY; y <= maxY; ++y) {
		for(int x = minX; x <= maxX; ++x) {

			//for each point we need to check if it lies inside the traingle or outside:
			float gamma = float((V2x-V1x)*(y-V1y) - (x-V1x)*(V2y-V1y))/float((V2x-V1x)*(V3y-V1y) - (V3x-V1x)*(V2y - V1y));
			float beta = float((V1x-V3x)*(y-V3y) - (x-V3x)*(V1y-V3y))/float((V2x-V1x)*(V3y-V1y) - (V3x-V1x)*(V2y - V1y));
			float alpha = (1.0 - beta - gamma);

			unsigned char r = v1.r*alpha + v2.r*beta + v3.r*gamma;
			unsigned char g = v1.g*alpha + v2.g*beta + v3.g*gamma;
			unsigned char b = v1.b*alpha + v2.b*beta + v3.b*gamma;

			if(alpha > -0.001 && alpha < 1 && gamma > -0.001 && gamma < 1 && beta > -0.001 && beta < 1) {
				image->setPixel(x, y, r, g, b);
			}
		}
	}

	// Write image to file
	image->writeToFile(filename);
	return 0;
}

