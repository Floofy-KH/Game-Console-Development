#include "EdgeGenerator.h"
#include "ColourAnalyser.h"
#include <iostream>

float obtainAspectRatio(CImg<int> image)
{
  int startX = image.width();
  int endX = 0;

  int startY = image.height();
  int endY = 0;

  for (int x = 10; x < image.width() - 1; x++)
  {
    for (int y = 10; y < image.height() - 10; y++)
    {
      //Since the image should have only one channel, trial and error suggests that grayValue is the pixel value while green is the alpha.
      int numchannels = image.spectrum();
      int grayValue = (int)image(x, y, 0, 0);
      //int greenValue = (int)image(x, y, 0, 1);

      //Might be better to have nested if statements. Not done so that I could check the values though.
      if (grayValue > 200 && x < startX)
      {
        startX = x;
      }

      if (grayValue > 200 && x > endX)
      {
        endX = x;
      }

      if (grayValue > 200 && y < startY)
      {
        startY = y;
      }

      if (grayValue > 200 && y > endY)
      {
        endY = y;
      }
    }
  }

  int appleWidth = endX - startX;
  int appleHeight = endY - startY;
  float aspectRatio = (float)appleHeight / (float)appleWidth;

  return aspectRatio;
}

int main(int argc, char** argv)
{
  int *edgeData = NULL;
  
  std::cout << "Loading image...\n";
  CImg<int> image ("apples/Cortland.bmp");
  std::cout << "Converting image to greyscale...\n";
	CImg<int> greyscale = CImg<int>(image);
	greyscale.channel(0);
	edgeData = new int[greyscale.size()];
	EdgeGenerator::generateEdges(greyscale.data(), greyscale.width(), greyscale.height(), 20, 40, edgeData);
	
	std::vector<int> redHist, greenHist, blueHist;
	ColourAnalyser::getHistograms(image, redHist, greenHist, blueHist, edgeData);
  redHist = ColourAnalyser::normaliseHistogram(redHist);
  greenHist = ColourAnalyser::normaliseHistogram(greenHist);
  blueHist = ColourAnalyser::normaliseHistogram(blueHist);

	std::cout << "Copying processed data for saving...\n";
	memcpy(image.data(), edgeData, image.size() * sizeof(int));
	delete[] edgeData;
	std::cout << "Saving image...\n";
	image.channel(0);
	image.save("processed.bmp");

	float AR = obtainAspectRatio(greyscale);
	std::cout << "aspect ratio is  " << AR << std::endl;

	std::cout << "Press any key to continue" << std::endl;
  std::cin.get();
}
