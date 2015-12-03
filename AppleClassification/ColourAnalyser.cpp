#include "ColourAnalyser.h"

#include <algorithm>
#include <assert.h>
#include <functional>
#include <iostream>

signature_tt<int> *currentSig1 = NULL, *currentSig2 = NULL;

int getCost(feature_tt *F1, feature_tt *F2)
{
  const int COST_MULT_FACTOR = 1000;
  const int THRESHOLD = 3 * COST_MULT_FACTOR;//1.412*COST_MULT_FACTOR;
  int first = currentSig1->Weights[*F1] - currentSig1->Weights[*F2];
  int second = currentSig2->Weights[*F1] - currentSig2->Weights[*F2];
  return std::min(THRESHOLD, static_cast<int>(COST_MULT_FACTOR*sqrt(first*first + second*second)));
}

//This code is taken from the examples of the FastEDM library and is copyright to Ofir Pele.
void ColourAnalyser::calculateSignatures(signature_tt<int> &sig1, signature_tt<int> &sig2, std::vector<int> hist1, std::vector<int> hist2)
{
  assert(hist1.size() == hist2.size());
  //-----------------------------------------------
  // Convert to FastEMD with Rubner's interface
  //-----------------------------------------------
  int size = (int)hist1.size();
  sig1.n = size;
  sig2.n = size;
  sig1.Features = new feature_tt[size];
  sig2.Features = new feature_tt[size];
  {for (int i = 0; i<size; ++i) {
    sig1.Features[i] = i;
    sig2.Features[i] = i;
  }}
  sig1.Weights = new int[size];
  sig2.Weights = new int[size];
  {for (int i = 0; i<size; ++i) {
    sig1.Weights[i] = hist1[i];
    sig2.Weights[i] = hist2[i];
  }}
  //-----------------------------------------------
}

int ColourAnalyser::getEMD(signature_tt<int> sig1, signature_tt<int> sig2)
{
  currentSig1 = &sig1;
  currentSig2 = &sig2;
  return emd_hat_signature_interface<int>(&sig1, &sig2, &getCost, -1);
}

//Return a vector containing the histogram of colour values. 
std::vector<int> ColourAnalyser::getHistogram(CImg<int> image, int channel, int *mask)
{
  std::vector<int> histogram;
  //Assign all colour counts to 0.
  histogram.assign(256, 0);

  for (int x = 0; x < image.width(); x++)
  {
    for (int y = 0; y < image.height(); y++)
    {
      int index = x + y*image.width();
      //If the mask isn't provided or if the value in the mask isn't black, add the pixel to the histogram. 
      if (mask == NULL|| (mask != NULL && mask[index] != 0))
      {
        int &value = image(x, y, 0, channel);
        histogram[value]++;
      }
    }
  }

  return histogram;
}

//Generate three histograms from an image, 1 for each colour (RGB). 
void ColourAnalyser::getHistograms(CImg<int> image, std::vector<int> &redHistogram, std::vector<int> &greenHistogram, std::vector<int> &blueHistogram, int *mask)
{
  //Assign all colour counts to 0.
  redHistogram.assign(256, 0);
  greenHistogram.assign(256, 0);
  blueHistogram.assign(256, 0);

  for (int x = 0; x < image.width(); x++)
  {
    for (int y = 0; y < image.height(); y++)
    {
      int index = x + y*image.width();
      //If the mask isn't provided or if the value in the mask isn't black, add the pixels to the histograms. 
      if (mask == NULL || (mask != NULL && mask[index] != 0))
      {
        int &value = image(x, y, 0, 0);
        redHistogram[value]++;
        value = image(x, y, 0, 1);
        greenHistogram[value]++;
        value = image(x, y, 0, 2);
        blueHistogram[value]++;
      }
    }
  }
}

//Returns a normalised histogram. 
const std::vector<int> ColourAnalyser::normaliseHistogram(const std::vector<int>& histogram)
{
  std::vector<int> cpy(histogram);
  float inverseSize = (1.f / cpy.size())*100; // Times by 100 to make the values percentages and keep as ints rather than floats.
  for (unsigned i = 0; i < cpy.size(); ++i)
  {
    cpy[i] = (int)(cpy[i] * inverseSize);
  }
  return cpy;
}

//Returns an averaged EMD value for the 2 images provided. 
const int ColourAnalyser::compareImages(CImg<int> image1, CImg<int> image2, int *mask1, int *mask2)
{
  std::cout << "Generating histograms for images..." << std::endl;
  std::vector<int> red1, red2, green1, green2, blue1, blue2;
  //Get the histograms for each image
  getHistograms(image1, red1, green1, blue1, mask1);
  getHistograms(image2, red2, green2, blue2, mask2);

  //Normalise all the histograms
  std::cout << "Normalising histograms..." << std::endl;
  red1 = normaliseHistogram(red1);
  red2 = normaliseHistogram(red2);
  green1 = normaliseHistogram(green1);
  green2 = normaliseHistogram(green2);
  blue1 = normaliseHistogram(blue1);
  blue2 = normaliseHistogram(blue2);

  //Create the signatures from the histograms.
  std::cout << "Creating signatures from histograms..." << std::endl;
  signature_tt<int> redSig1, redSig2, greenSig1, greenSig2, blueSig1, blueSig2;
  calculateSignatures(redSig1, redSig2, red1, red2);
  calculateSignatures(greenSig1, greenSig2, green1, green2);
  calculateSignatures(blueSig1, blueSig2, blue1, blue2);

  //Calculate the EMD for each colour
  std::cout << "Calculating EDM for red..." << std::endl;
  int redEMD = getEMD(redSig1, redSig2);
  std::cout << "Calculating EDM for green..." << std::endl;
  int greenEMD = getEMD(greenSig1, greenSig2);
  std::cout << "Calculating EDM for blue..." << std::endl;
  int blueEMD = getEMD(blueSig1, blueSig2);

  //Average the three values to get an average EMD for the image and return it
  std::cout << "Averaging EDM..." << std::endl;
  return (redEMD + greenEMD + blueEMD) / 3;
}
