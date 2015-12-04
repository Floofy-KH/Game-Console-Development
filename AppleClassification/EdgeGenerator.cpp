#include "EdgeGenerator.h"
#include "SPEContextManager.h"
#include <assert.h>
#include <math.h>
#include <iostream>
#include <utility>
#include <vector>

void EdgeGenerator::applyKernal(int * inData, int rowStride, int row, int col, int maxRows, int maxCols, int * outData, float * kernal, int kernalWidth, int kernalHeight)
{
	assert(kernalWidth % 2 != 0);
	assert(kernalHeight % 2 != 0);

	int kernalWidthExtent = (int)floor(kernalWidth / 2.0f);
	int kernalHeightExtent = (int)floor(kernalHeight / 2.0f);

	bool validData = (inData && outData && kernal);
	bool validIndex = (row >= kernalHeightExtent && row < maxRows - kernalHeightExtent && col >= kernalWidthExtent && col < maxCols - kernalWidthExtent);
	if (validData)
	{
		float result = 0;
		if (validIndex)
		{
			for (int x = -kernalWidthExtent; x <= kernalWidthExtent; ++x)
			{
				for (int y = -kernalHeightExtent; y <= kernalHeightExtent; ++y)
				{
					int dataIndex = (col + x) + (row + y)*rowStride;
					int kernalIndex = (x + kernalWidthExtent) + (y + kernalHeightExtent) * kernalWidth;
					result += inData[dataIndex] * kernal[kernalIndex];
				}
			}
		}
		else
		{
			result = (float)inData[col + row*rowStride];
		}
		outData[col + row*rowStride] = (int)result;
	}
	else
	{
		std::cerr << "Invalid data provided" << std::endl;
	}
}

void EdgeGenerator::calculateEdgeDirections(int * inXData, int * inYData, float * outData, int size)
{
	for (int i = 0; i < size; ++i)
	{
		if (inXData[i] == 0)
		{
			outData[i] = (inYData[i] == 0) ? 0.0f : 90.0f;
		}
		else
		{
			outData[i] = (float)atan(abs(inYData[i]) / (float)abs(inXData[i]));
		}

		if (outData[i] <= 112.5f && outData[i] >= 67.5f)
		{
			outData[i] = 90.0f;
		}
		else if (outData[i] <= 157.5f && outData[i] > 112.5f)
		{
			outData[i] = 135.0f;
		}
		else if (outData[i] < 67.5f && outData[i] >= 22.5f)
		{
			outData[i] = 45.0f;
		}
		else if ((outData[i] < 22.5f && outData[i] >= 0.0f) || (outData[i] <= 180.0f && outData[i] > 157.5f))
		{
			outData[i] = 0.0f;
		}
		else
		{
			std::cerr << "Invalid edge direction: " << outData[i] << "\n";
			outData[i] = 0.0f;
		}
	}
}

void EdgeGenerator::performNonMaximumSuppression(int * data, float * directions, int width, int height)
{
	for (int col = 0; col < width; ++col)
	{
		for (int row = 0; row < height; ++row)
		{
			int index = col + row*width;
			int size = width*height;
			if (directions[index] == 90.0f)
			{
				int oneUpIndex = col + (row - 1)*width;
				int oneDownIndex = col + (row + 1)*width;
				if ((oneUpIndex > 0 && oneDownIndex < size) && (data[index] < data[oneDownIndex] || data[index] < data[oneUpIndex]))
				{
					data[index] = 0;
				}
			}
			else if (directions[index] == 0.0f)
			{
				int oneRightIndex = (col + 1) + row*width;
				int oneLeftIndex = (col - 1) + row*width;
				if ((oneLeftIndex > 0 && oneRightIndex < size) && (data[index] < data[oneLeftIndex] || data[index] < data[oneRightIndex]))
				{
					data[index] = 0;
				}
			}
			else if (directions[index] == 45.0f)
			{
				int oneUpRightIndex = (col + 1) + (row - 1)*width;
				int oneDownLeftIndex = (col - 1) + (row + 1)*width;
				if ((oneUpRightIndex > 0 && oneUpRightIndex < size && oneDownLeftIndex > 0 && oneDownLeftIndex < size) && (data[index] < data[oneDownLeftIndex] || data[index] < data[oneUpRightIndex]))
				{
					data[index] = 0;
				}
			}
			else if (directions[index] == 135.0f)
			{
				int oneUpLeftIndex = (col - 1) + (row - 1)*width;
				int oneDownRightIndex = (col + 1) + (row + 1)*width;
				if ((oneUpLeftIndex > 0 && oneUpLeftIndex < size && oneDownRightIndex > 0 && oneDownRightIndex < size) && (data[index] < data[oneDownRightIndex] || data[index] < data[oneUpLeftIndex]))
				{
					data[index] = 0;
				}
			}
			else
			{
				std::cerr << "Invalid direction: " << directions[index] << std::endl;
			}
		}
	}
}

void EdgeGenerator::applyThresholding(int * data, int size, int lowThreshold, int highThreshold, int strongEdgeValue, int weakEdgeValue)
{
  spe_context_ptr_t context;
  spe_program_handle_t *speImage;
  spe_stop_info_t stopInfo;
  unsigned int entry = SPE_DEFAULT_ENTRY;
  SPEContextManager speManager;
  speManager.initialise();
  speImage = speManager.getSPEImage("SPECode/ApplyThresholding");
  context = speManager.createContext();

  static int argumentData[16] = {
    (int)data, size, lowThreshold, highThreshold, strongEdgeValue, weakEdgeValue,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  } __attribute__((aligned(64)));

  if (speManager.loadProgramHandle(context, speImage))
  {
    speManager.runSPEContext(context, &stopInfo, &entry, SPE_RUN_USER_REGS, &argumentData, 0);
  }
  speManager.closeSPEImage(speImage);
  speManager.destroyContext(context);

	/*for (int i = 0; i < size; ++i)
	{
		if (data[i] > highThreshold)
		{
			data[i] = strongEdgeValue;
		}
		else if (data[i] < lowThreshold)
		{
			data[i] = 0;
		}
		else
		{
			data[i] = weakEdgeValue;
		}
	}*/
}

void EdgeGenerator::applyHysteresisTracking(int * inData, int width, int height, int strongEdgeValue, int * outData)
{
	for (int col = 0; col < width; ++col)
	{
		for (int row = 0; row < height; ++row)
		{
			int centre = col + row*width;

			if (inData[centre] != strongEdgeValue)
			{
				bool strongEdgePresent = false;
				for (int colOffset = -1; colOffset <= 1; ++colOffset)
				{
					for (int rowOffset = -1; rowOffset <= 1; ++rowOffset)
					{
						int outer = col + colOffset + (row + rowOffset) * width;
						if (outer > 0 && outer < width * height)
						{
							if (inData[outer] == strongEdgeValue)
							{
								strongEdgePresent = true;
							}
						}
					}
				}
				outData[centre] = (strongEdgePresent) ? inData[centre] : 0;
			}
			else
			{
				outData[centre] = inData[centre];
			}
		}
	}
}

void EdgeGenerator::fillBoundaries(int * inData, int width, int height, int strongEdgeValue, int weakEdgeValue, int * outData)
{
	const int ignoreOffset = 10;
  std::vector< std::pair<int, int> > boundaries(height);
  
	for (int row = 0; row < height; ++row)
	{
    bool firstEdgeFound = false;
		for (int col = 0; col < width; ++col)
		{
			int i = col + row*width;
			if (col > ignoreOffset && col < width - ignoreOffset && row > ignoreOffset && row < height - ignoreOffset)
			{
        if ((inData[i] == strongEdgeValue || inData[i] == weakEdgeValue))
        {
          if (!firstEdgeFound)
          {
            firstEdgeFound = true;
            boundaries[row] = std::pair<int, int>(col, 0);
          }
          else
          {
            boundaries[row].second = col;
          }
        }
			}
		}
	}

  for (int row = 0; row < height; ++row)
  {
    std::pair<int, int> boundary = boundaries[row];
    for (int col = 0; col < width; ++col)
    {
      int i = col + row*width;
      if (boundary.second != 0 && col > boundary.first && col < boundary.second)
      {
        outData[i] = 255;
      }
      else
      {
        outData[i] = 0; 
      }
    }
  }
}

void EdgeGenerator::generateEdges(int * inData, int width, int height, int lowThreshold, int highThreshold, int * outData)
{
	const int STRONG_EDGE_VALUE = 255;
	const int WEAK_EDGE_VALUE = 100;

	float Gx[] = { 1, 2, 1, 0, 0, 0, -1, -2, -1 };
	float Gy[] = { 1, 0, -1, 2, 0, -2, 1, 0, -1 };

	float GaussianBlur[] =
	{
		2, 4, 5, 4, 2,
		4, 9, 12, 9, 4,
		5, 12, 15, 12, 2,
		4, 9, 12, 9, 4,
		2, 4, 5, 4, 2,
	};
	float blurThingy = 1 / 159.f; //Maybe find a better name for this (when I find out what it is exactly).
	for (int i = 0; i < 25; ++i)
	{
		GaussianBlur[i] *= blurThingy;
	}

	int *guassianResult = NULL;
	int *sobelResultX = NULL;
	int *sobelResultY = NULL;
	int *sobelResult = NULL;
	int *hysteresisResult = NULL;
	int *boundaryResult = NULL;
	float *directions = NULL;
	int imageSize = width*height;
	
	guassianResult = new int[imageSize];
	sobelResult = new int[imageSize];
	sobelResultX = new int[imageSize];
	sobelResultY = new int[imageSize];
	hysteresisResult = new int[imageSize];
	boundaryResult = new int[imageSize];
	directions = new float[imageSize];

	std::cout << "Applying Gaussian filter...\n";
	for (int i = 0; i < 1; ++i)
	{
		for (int row = 0; row<height; ++row)
		{
			for (int col = 0; col<width; ++col)
			{
				applyKernal(inData, width, row, col, height, width, guassianResult, GaussianBlur, 5, 5);
			}
		}
	}

	std::cout << "Applying Sobel filters...\n";
	for (int row = 0; row<height; ++row)
	{
		for (int col = 0; col<width; ++col)
		{
      applyKernal(guassianResult, width, row, col, height, width, sobelResultX, Gx, 3, 3);
      applyKernal(guassianResult, width, row, col, height, width, sobelResultY, Gy, 3, 3);
			int index = col + row*width;
			sobelResult[index] = (int)sqrt(sobelResultX[index] * sobelResultX[index] + sobelResultY[index] * sobelResultY[index]);
		}
	}

	std::cout << "Calculating edge directions...\n";
	calculateEdgeDirections(sobelResultX, sobelResultY, directions, imageSize);
	delete[] sobelResultX;
	delete[] sobelResultY;

	std::cout << "Performing non-maximum suppression...\n";
	performNonMaximumSuppression(sobelResult, directions, width, height);
	delete[] directions;

	std::cout << "Applying double thresholding...\n";
	applyThresholding(sobelResult, imageSize, lowThreshold, highThreshold, STRONG_EDGE_VALUE, WEAK_EDGE_VALUE);

	std::cout << "Performing edge tracking with hysteresis...\n";
	applyHysteresisTracking(sobelResult, width, height, STRONG_EDGE_VALUE, hysteresisResult);
	delete[] sobelResult;

	std::cout << "Filling image boundaries...\n";
	fillBoundaries(hysteresisResult, width, height, STRONG_EDGE_VALUE, WEAK_EDGE_VALUE, boundaryResult);
	delete[] hysteresisResult;

  memcpy(outData, boundaryResult, imageSize * sizeof(int));
	delete[] boundaryResult;
}
