#include "ColourAnalyser.h"

#include <algorithm>
#include <assert.h>
#include <functional>

signature_tt<int> *currentSig1 = nullptr, *currentSig2 = nullptr;

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

//This code is taken from the examples of the FastEDM library and is copyright to Ofir Pele.
Vector2D ColourAnalyser::generateCostMatrix(int width, int height)
{
  Vector2D costMatrix;
  const int COST_MULT_FACTOR = 1000;
  const int THRESHOLD = 3 * COST_MULT_FACTOR;//1.412*COST_MULT_FACTOR;
  // The ground distance - thresholded Euclidean distance.
  // Since everything is ints, we multiply by COST_MULT_FACTOR.
  // std::vector< std::vector<int> > cost_mat; // here it's defined as global for Rubner's interfaces.
  // If you do not use Rubner's interface it's a better idea
  // not to use globals.
  std::vector<int> cost_mat_row(height*width);
  for (int i = 0; i<height*width; ++i) costMatrix.push_back(cost_mat_row);
  int max_cost_mat = -1;
  int j = -1;
  for (int c1 = 0; c1<width; ++c1) {
    for (int r1 = 0; r1<height; ++r1) {
      ++j;
      int i = -1;
      for (int c2 = 0; c2<width; ++c2) {
        for (int r2 = 0; r2<height; ++r2) {
          ++i;
          costMatrix[i][j] = std::min(THRESHOLD, static_cast<int>(COST_MULT_FACTOR*sqrt((r1 - r2)*(r1 - r2) + (c1 - c2)*(c1 - c2))));
        }
      }
    }
  }

  return costMatrix;
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
      if (mask == nullptr || (mask != nullptr && mask[index] != 0))
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
      if (mask == nullptr || (mask != nullptr && mask[index] != 0))
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
  std::vector<int> red1, red2, green1, green2, blue1, blue2;
  //Get the histograms for each image
  getHistograms(image1, red1, green1, blue1, mask1);
  getHistograms(image2, red2, green2, blue2, mask2);

  //Normalise all the histograms
  red1 = normaliseHistogram(red1);
  red2 = normaliseHistogram(red2);
  green1 = normaliseHistogram(green1);
  green2 = normaliseHistogram(green2);
  blue1 = normaliseHistogram(blue1);
  blue2 = normaliseHistogram(blue2);

  //Generate the cost matrix used in the EMD calculation. 
  //Vector2D costMatrix = generateCostMatrix(image1.width(), image2.height());

  //Create the signatures from the histograms.
  signature_tt<int> redSig1, redSig2, greenSig1, greenSig2, blueSig1, blueSig2;
  calculateSignatures(redSig1, redSig2, red1, red2);
  calculateSignatures(greenSig1, greenSig2, green1, green2);
  calculateSignatures(blueSig1, blueSig2, blue1, blue2);

  //Calculate the EMD for each colour
  int redEMD = getEMD(redSig1, redSig2);
  int greenEMD = getEMD(greenSig1, greenSig2);
  int blueEMD = getEMD(blueSig1, blueSig2);

  //Average the three values to get an average EMD for the image and return it
  return (redEMD + greenEMD + blueEMD) / 3;
}
