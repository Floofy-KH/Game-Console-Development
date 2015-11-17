#include "ColourAnalyser.h"

std::vector<int> ColourAnalyser::getHistogram(CImg<int> image, int channel, int *mask)
{
  std::vector<int> histogram;
  histogram.assign(256, 0);

  for (int x = 0; x < image.width(); x++)
  {
    for (int y = 0; y < image.height(); y++)
    {
      int index = x + y*image.width();
      if (mask == nullptr || (mask == nullptr && mask[index] != 0))
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
      if (mask == nullptr || (mask == nullptr && mask[index] != 0))
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