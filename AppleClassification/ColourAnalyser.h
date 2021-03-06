#pragma once
#pragma warning(push, 0)
#include "CImg.h"
#include "FastEMD/emd_hat_signatures_interface.hpp"
#pragma warning(pop)
#include <vector>
using namespace cimg_library;

typedef std::vector< std::vector<int> > Vector2D;

class ColourAnalyser
{
public:
  ~ColourAnalyser(){};
  static std::vector<int> getHistogram(CImg<int> image, int channel, int *mask = NULL);
  static void getHistograms(CImg<int> image, std::vector<int> &redHistogram, std::vector<int> &greenHistogram, std::vector<int> &blueHistogram, int *mask = NULL);
  static const std::vector<int> normaliseHistogram(const std::vector<int>& histogram);
  static const int compareImages(CImg<int> image1, CImg<int> image2, int *mask1 = NULL, int *mask2 = NULL);

private:
  ColourAnalyser(){};
  ColourAnalyser(const ColourAnalyser&) {}

  static int getEMD(signature_tt<int> sig1, signature_tt<int> sig2);
  static Vector2D generateCostMatrix(int width, int height);
  static void calculateSignatures(signature_tt<int> &sig1, signature_tt<int> &sig2, std::vector<int> hist1, std::vector<int> hist2);

  const ColourAnalyser operator= (const ColourAnalyser&) {}

};

