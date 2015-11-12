#pragma once
class EdgeGenerator
{
private:
	EdgeGenerator() {}
	EdgeGenerator(const EdgeGenerator&) {}
	const EdgeGenerator operator= (const EdgeGenerator&) {}

	static void applyKernal(int *inData, int rowStride, int row, int col, int maxRows, int maxCols, int *outData, float *kernal, int kernalWidth, int kernalHeight);
public:
	
	static void calculateEdgeDirections(int *inXData, int *inYData, float *outData, int size);
	static void performNonMaximumSuppression(int *data, float *directions, int width, int height);
	static void applyThresholding(int *data, int size, int lowThreshold, int highThreshold, int strongEdgeValue, int weakEdgeValue);
	static void applyHysteresisTracking(int *inData, int width, int height, int strongEdgeValue, int *outData);
	static void fillBoundaries(int *inData, int width, int height, int strongEdgeValue, int weakEdgeValue, int *outData);
	static void generateEdges(int *inData, int width, int height, int lowThreshold, int highThreshold, int *outData);

	~EdgeGenerator() {}
};

