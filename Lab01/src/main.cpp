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

int main(int argc, char **argv)
{
	if(argc < 9) {
		cout << "Usage: Lab1 <out_image_name>.png width height" << endl;
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

	// Draw a rectangle
	for(int y = minY; y <= maxY; ++y) {
		for(int x = minX; x <= maxX; ++x) {
			unsigned char r = 5;
			unsigned char g = 40;
			unsigned char b = 80;
			image->setPixel(x, y, r, g, b);
		}
	}

	//set the vertex points
	image->setPixel(V1x, V1y, 90, 30, 40);
	image->setPixel(V2x, V2y, 90, 30, 40);
	image->setPixel(V3x, V3y, 90, 30, 40);

	// Write image to file
	image->writeToFile(filename);
	return 0;
}
