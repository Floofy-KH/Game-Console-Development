#pragma once
#include "CImg.h"
#include <vector>
using namespace cimg_library;

class ColourAnalyser
{
public:
  ~ColourAnalyser(){};
  std::vector<int> getHistogram(CImg<int> image, int channel, int *mask = nullptr);
  void getHistograms(CImg<int> image, std::vector<int> &redHistogram, std::vector<int> &greenHistogram, std::vector<int> &blueHistogram, int *mask = nullptr);

private:
  ColourAnalyser(){};
  ColourAnalyser(const ColourAnalyser&) {}
  const ColourAnalyser operator= (const ColourAnalyser&) {}

};

