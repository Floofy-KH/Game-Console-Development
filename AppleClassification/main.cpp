#include "EdgeGenerator.h"
#include "ColourAnalyser.h"
#include "SPEContextManager.h"
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

struct stuff
{
  int numbers[31];
  int sum;
};

int main(int argc, char** argv)
{
  spe_context_ptr_t context;
  spe_program_handle_t *speImage;
  spe_stop_info_t stopInfo;
  SPEContextManager speManager;
  speManager.initialise();
  speImage = speManager.getSPEImage("SPECode/AddNumbersSPE");
  context = speManager.createContext();

  int *edgeData1 = NULL, *edgeData2 = NULL;

  static stuff thing __attribute__ ((aligned(128)));
  
  for(int i=0; i<31; i++)
  { 
    thing.numbers[i] = i+1;
  } 
  
  std::cout << "Loading images...\n";
  CImg<int> image1 ("apples/Cortland.bmp");
  CImg<int> image2 ("apples/Cortland.bmp");

  if (speManager.loadProgramHandle(context, speImage))
  {
    speManager.runSPEContext(context, &stopInfo, &thing);
  }
  speManager.closeSPEImage(speImage);
  speManager.destroyContext(context);

  for (int i = 0; i < 31; ++i)
  {
    std::cout << thing.numbers[i];
    if (i != 30)
    {
      std::cout << " + ";
    }
  }
  std::cout << " = " << thing.sum << std::endl;

  std::cout << "Converting images to greyscale...\n";
	CImg<int> greyscale1 = CImg<int>(image1);
  CImg<int> greyscale2 = CImg<int>(image2);
	greyscale1.channel(0);
  greyscale2.channel(0);
	edgeData1 = new int[greyscale1.size()];
  edgeData2 = new int[greyscale2.size()];
	EdgeGenerator::generateEdges(greyscale1.data(), greyscale1.width(), greyscale1.height(), 20, 40, edgeData1);
  EdgeGenerator::generateEdges(greyscale2.data(), greyscale2.width(), greyscale2.height(), 20, 40, edgeData2);
	
  int EMD = ColourAnalyser::compareImages(image1, image2, edgeData1, edgeData2);
  std::cout << "EMD is: " << EMD << std::endl;

	std::cout << "Press any key to continue" << std::endl;
  std::cin.get();
}
