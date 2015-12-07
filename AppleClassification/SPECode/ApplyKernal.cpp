#include <spu_mfcio.h>
#include <math.h>
#include <stdio.h>

#define CHUNK_SIZE 64
#define MAX_SIZE (CHUNK_SIZE*CHUNK_SIZE)
#define LOGGING 1

#define KERNAL_TAG 2

#if LOGGING
  #define DPRINTF printf
#else
  #define DPRINTF
#endif

//Globals
unsigned int inDataAddress, outDataAddress, width, height, kernalAddress, kernalSize, size;
mfc_list_element_t getElements[CHUNK_SIZE], putElements[CHUNK_SIZE];
int inData[MAX_SIZE * 2] __attribute__((aligned(128))), outData[MAX_SIZE * 2] __attribute__((aligned(128)));
float *kernal;

inline bool putChunk(int chunkIndex)
{
  bool validChunk = true;

  DPRINTF("Checking chunk index\n", chunkIndex);
  int col = 0, row = 0;

  for (int i = 0; i < chunkIndex; ++i)
  {
    col += chunkIndex*(CHUNK_SIZE - kernalSize);
    if (col + CHUNK_SIZE > width) //If this chunk would go beyond the width of the image, partial chunk still needs processed
    {
      col = width - CHUNK_SIZE;
    }
    else if (col + CHUNK_SIZE == width) //If this chunk would end at the width, move to next row;
    {
      col = 0;
      row += CHUNK_SIZE;

      if (row + CHUNK_SIZE > height) //If this chunk would go beyond the width of the image, partial chunk still needs processed
      {
        row = height - CHUNK_SIZE;
      }
      else if (row + CHUNK_SIZE == height)
      {
        DPRINTF("Chunk %d goes out of range\n", chunkIndex);
        validChunk = false;
      }
    }
  }

  if (validChunk)
  {
    DPRINTF("Getting %dth data chunk\n", chunkIndex);
    for (unsigned int i = 0; i<CHUNK_SIZE; ++i)
    {
      putElements[i].size = CHUNK_SIZE;
      putElements[i].eal = outDataAddress + col + (row + i)*width;
    }

    int bufferPos = chunkIndex & 1;
    mfc_putl(outData, outDataAddress + bufferPos*MAX_SIZE, putElements, sizeof(putElements), bufferPos, 0, 0);
  }

  return validChunk;
}

inline bool getChunk(int chunkIndex)
{
  bool validChunk = true;

  DPRINTF("Checking chunk index\n", chunkIndex);
  int col = 0, row = 0;

  for (int i = 0; i < chunkIndex; ++i)
  {
    col += chunkIndex*(CHUNK_SIZE - kernalSize);
    if (col + CHUNK_SIZE > width) //If this chunk would go beyond the width of the image, partial chunk still needs processed
    {
      col = width - CHUNK_SIZE;
    }
    else if (col + CHUNK_SIZE == width) //If this chunk would end at the width, move to next row;
    {
      col = 0;
      row += CHUNK_SIZE;

      if (row + CHUNK_SIZE > height) //If this chunk would go beyond the width of the image, partial chunk still needs processed
      {
        row = height - CHUNK_SIZE;
      }
      else if (row + CHUNK_SIZE == height)
      {
        DPRINTF("Chunk %d goes out of range\n", chunkIndex);
        validChunk = false;
      }
    }
  }

  if (validChunk)
  {
    DPRINTF("Getting %dth data chunk\n", chunkIndex);
    for (unsigned int i = 0; i<CHUNK_SIZE; ++i)
    {
      getElements[i].size = CHUNK_SIZE;
      getElements[i].eal = inDataAddress + col + (row + i)*width;
    }

    int bufferPos = chunkIndex & 1;
    mfc_getl(inData, inDataAddress + bufferPos*MAX_SIZE, getElements, sizeof(getElements), bufferPos, 0, 0);
  }

  return validChunk;
}

inline int processPixel(int col, int row)
{
  int kernalExtent = (int)floor(kernalSize / 2.0f);
  int result = 0;

  for (int x = -kernalExtent; x <= kernalExtent; ++x)
  {
    for (int y = -kernalExtent; y <= kernalExtent; ++y)
    {
      int dataIndex = (col + x) + (row + y)*CHUNK_SIZE;
      int kernalIndex = (x + kernalExtent) + (y + kernalExtent) * kernalSize;
      result += inData[dataIndex] * kernal[kernalIndex];
    }
  }

  return result;
}

inline void processChunk(int bufferPos)
{
  for (int row = kernalSize; row < CHUNK_SIZE - kernalSize; ++row)
  {
    for (int col = kernalSize; col < CHUNK_SIZE - kernalSize; ++col)
    {
      int outIndex = col + row*CHUNK_SIZE;
      outData[bufferPos*MAX_SIZE + outIndex] = processPixel(col, row);
    }
  }
}

int main(vector unsigned int arg1, vector unsigned int arg2, vector unsigned int arg3)
{
  DPRINTF("Extracting parameters.\n");
  inDataAddress = spu_extract(arg1, 0);
  outDataAddress = spu_extract(arg1, 1);
  width = spu_extract(arg1, 2);
  height = spu_extract(arg1, 2);
  kernalAddress = spu_extract(arg2, 0);
  kernalSize = spu_extract(arg2, 1);
  size = width*height;
  DPRINTF("In data: %u\nOut data: %u\nWidth: %u\nHeight: %u\nKernal: %u\nKernal size: %u\n", inDataAddress, outDataAddress, width, height, kernalAddress, kernalSize);

  mfc_list_element_t getElements[CHUNK_SIZE], putElements[CHUNK_SIZE];
  int inData[MAX_SIZE*2] __attribute__((aligned(128))), outData[MAX_SIZE*2] __attribute__((aligned(128)));
  float kernalArray[kernalSize*kernalSize] __attribute__((aligned(128)));
  kernal = kernalArray;

  DPRINTF("Getting kernal data\n");
  mfc_get(kernal, kernalAddress, kernalSize*kernalSize, KERNAL_TAG, 0, 0);

  DPRINTF("Geting first data chunk\n");
  int chunkIndex = 0;
  getChunk(chunkIndex++);

  while (getChunk(chunkIndex++))
  {
    int bufferPos = chunkIndex & 1;

    DPRINTF("Wait for %dth chunk to be transferred\n", chunkIndex - 1);
    mfc_write_tag_mask(1 << (1 - bufferPos) | KERNAL_TAG);
    mfc_read_tag_status_all();

    DPRINTF("Proces the %dth chunk\n", chunkIndex - 1);
    processChunk(1 - bufferPos);

    DPRINTF("Sending processed chunk %d back to main memory\n", chunkIndex - 1);
    putChunk(chunkIndex - 1);
  }

  DPRINTF("Wait for last data transfer to complere\n");
  mfc_write_tag_mask(2);
  mfc_read_tag_status_all();

  DPRINTF("Doing final chunk processing\n");
  processChunk(chunkIndex - 1);

  DPRINTF("Sending final data back to main memory\n");
  putChunk(chunkIndex - 1);
  mfc_read_tag_status_all();
}
