#include "ColourAnalyser.h"

#include <algorithm>
#include <assert.h>
#include <functional>

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
  // The ground distance - thresholded Euclidean distance.
  // Since everything is ints, we multiply by COST_MULT_FACTOR.
  const int COST_MULT_FACTOR = 1000;
  const int THRESHOLD = 3 * COST_MULT_FACTOR;//1.412*COST_MULT_FACTOR;
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

int ColourAnalyser::getEMD(signature_tt<int> sig1, signature_tt<int> sig2, const Vector2D &costMatrix)
{
  std::function<int(feature_tt*, feature_tt*)> distFunc = [&](feature_tt *F1, feature_tt *F2) {return (costMatrix)[*F1][*F2]; };

  return emd_hat_signature_interface<int>(&sig1, &sig2, *distFunc.target<int(*)(feature_tt*, feature_tt*)>(), -1);
}

std::vector<int> ColourAnalyser::getHistogram(CImg<int> image, int channel, int *mask)
{
  std::vector<int> histogram;
  histogram.assign(256, 0);

  for (int x = 0; x < image.width(); x++)
  {
    for (int y = 0; y < image.height(); y++)
    {
      int index = x + y*image.width();
      if (mask == nullptr || (mask != nullptr && mask[index] != 0))
      {
        int &value = image(x, y, 0, channel);
        histogram[value]++;
      }
    }
  }

  return histogram;
}

void ColourAnalyser::getHistograms(CImg<int> image, std::vector<int> &redHistogram, std::vector<int> &greenHistogram, std::vector<int> &blueHistogram, int *mask)
{
  redHistogram.assign(256, 0);
  greenHistogram.assign(256, 0);
  blueHistogram.assign(256, 0);

  for (int x = 0; x < image.width(); x++)
  {
    for (int y = 0; y < image.height(); y++)
    {
      int index = x + y*image.width();
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


const std::vector<int> ColourAnalyser::normaliseHistogram(const std::vector<int>& histogram)
{
  std::vector<int> cpy(histogram);
  float inverseSize = (1.f / cpy.size())*100; // Times by 100 to make the values percentages.
  for (unsigned i = 0; i < cpy.size(); ++i)
  {
    cpy[i] = (int)(cpy[i] * inverseSize);
  }
  return cpy;
}