#include "EdgeGenerator.h"
#include "ColourAnalyser.h"
#include "SPEContextManager.h"
#include <iostream>
#include <pthread.h>
#include <math.h>

struct ThreadData
{
  int *edgeData;
  CImg<int> *image;
  const char *fileName;
};


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

void resizeToPow2(CImg<int> *img)
{
  bool resizeWidth = (img->width() & (img->width() - 1)) != 0;
  bool resizeHeight = (img->height() & (img->height() - 1)) != 0;
  int newWidth = img->width();
  int newHeight = img->height();
  if(resizeWidth)
  {
    newWidth = pow(2, ceil(log(img->width()) / log(2))); 
  }
  if(resizeHeight)
  {
    newHeight = pow(2, ceil(log(img->height()) / log(2))); 
  }
  
  img->resize(newWidth, newHeight, img->depth(), img->spectrum(), -1);
}

void* mainThreadFunc(void *data)
{
  ThreadData *threadData = (ThreadData*)data;
  int *edgeData = threadData->edgeData;
  CImg<int> *image = threadData->image;

  std::cout << "Loading imags...\n";
  image = new CImg<int> (threadData->fileName);
  resizeToPow2(image);

  std::cout << "Converting image to greyscale...\n";
	CImg<int> greyscale = CImg<int>(*image);
	greyscale.channel(0);
	edgeData = new int[greyscale.size()];
	EdgeGenerator::generateEdges(greyscale.data(), greyscale.width(), greyscale.height(), 20, 40, edgeData);
}

int main(int argc, char** argv)
{
  ThreadData data1, data2;
  int *edgeData1 = NULL, *edgeData2 = NULL;
  CImg<int> *image1, *image2;
  data1.edgeData = edgeData1;
  data1.image = image1;
  data1.fileName = "apples/Gala.bmp";
  data2.edgeData = edgeData2;
  data2.image = image2;
  data2.fileName = "apples/Jonathan.bmp";

  pthread_t thread1, thread2;
  pthread_create(&thread1, NULL, &mainThreadFunc, &data1);
  pthread_create(&thread2, NULL, &mainThreadFunc, &data2);
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  int EMD = ColourAnalyser::compareImages(*image1, *image2, edgeData1, edgeData2);
  std::cout << "EMD is: " << EMD << std::endl;

	std::cout << "Press any key to continue" << std::endl;
  std::cin.get();
}
