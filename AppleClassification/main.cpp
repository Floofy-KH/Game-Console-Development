#include "CImg.h"
#include "EdgeGenerator.h"
#include <iostream>
using namespace cimg_library;

int main(int argc, char** argv)
{
  int *edgeData = NULL;
  
  std::cout << "Loading image...\n";
  CImg<int> image ("apple.bmp");
  std::cout << "Converting image to greyscale...\n";
  image.channel(0);
	edgeData = new int[image.size()];
	EdgeGenerator::generateEdges(image.data(), image.width(), image.height(), 20, 40, edgeData);

  std::cout << "Copying processed data for saving...\n";
  memcpy(image.data(), edgeData, image.size() * sizeof(int));
	delete[] edgeData;

  std::cout << "Saving image...\n";
  image.save("processed.bmp");
}
