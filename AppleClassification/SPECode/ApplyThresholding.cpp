#include <spu_mfcio.h>
#include <stdio.h>

#define MAX_SIZE 512 
#define LOGGING 1

int main(vector unsigned int arg1,
vector unsigned int arg2, 
vector unsigned int arg3)
{
  printf("Reading in argument variables\n");
  unsigned int dataAddress = spu_extract(arg1, 1); 
  unsigned int size = spu_extract(arg1, 0); 
  unsigned int lowThreshold = spu_extract(arg1, 2); 
  unsigned int highThreshold = spu_extract(arg1, 3); 
  unsigned int strongEdgeValue = spu_extract(arg2, 0); 
  unsigned int weakEdgeValue = spu_extract(arg2, 1); 
  printf("Argument variables are:\ndataAddress:%u\nsize:%d\nthresholds:%d-%d\nedgeValues%d-%d\n", dataAddress, size, lowThreshold, highThreshold, strongEdgeValue, weakEdgeValue); 

  int dataBuffer[MAX_SIZE*2] __attribute__((aligned(128)));
  int blockSize = sizeof(dataBuffer) / 2;
  int start = 0, end = 0;

  int iterations = size / MAX_SIZE;

  printf("Beginning double-buffered memory transfers and data processing, %d iterations required\n", iterations);
  if (iterations > 1)
  {
    printf("Getting first memory block with tag 0\n");
    mfc_get(dataBuffer, (int*)dataAddress, blockSize, 0, 0, 0);
    for (int i = 1; i < iterations; ++i)
    {
      printf("Getting %dth memory block using tag %d\n", (i+1), i&1);
      mfc_get(dataBuffer + (i & 1)*MAX_SIZE, dataAddress + i*blockSize, blockSize, i&1, 0, 0);
      
      printf("Waiting for memory block transfer with tag %d to complete\n", 1-(i&1));
      mfc_write_tag_mask(1 << (1 - (i & 1)));
      mfc_read_tag_status_all();
      printf("Previous transfer complete, coninuing processing\n");      

      start = (i & 1) ? 0 : MAX_SIZE;
      end = start + MAX_SIZE;
      printf("Applying thresholding to current data block\n");
      for (int j = start; j < end; ++j)
      {
        if (dataBuffer[j] > highThreshold)
        {
          dataBuffer[j] = strongEdgeValue;
        }
        else if (dataBuffer[j] < lowThreshold)
        {
          dataBuffer[j] = 0;
        }
        else
        {
          dataBuffer[j] = weakEdgeValue;
        }
      }
      printf("Sending processed datablock back to main memory with tag %d\n", 1-(i&1));
      mfc_put(dataBuffer + (1 - (i & 1))*MAX_SIZE, dataAddress + (i - 1)*blockSize, blockSize, 1 - (i & 1), 0, 0);
    }
    printf("Waiting for last data transfer to complete\n");
    mfc_write_tag_mask(2);
    mfc_read_tag_status_all();
    printf("Last data transfer complete, doing final thresholding for complete block.\n");
    start = MAX_SIZE;
    end = 2 * MAX_SIZE;
    for (int j = start; j < end; ++j)
    {
      if (dataBuffer[j] > highThreshold)
      {
        dataBuffer[j] = strongEdgeValue;
      }
      else if (dataBuffer[j] < lowThreshold)
      {
        dataBuffer[j] = 0;
      }
      else
      {
        dataBuffer[j] = weakEdgeValue;
      }
    }
    printf("Sendin processed data back to main memory\n");
    mfc_put(dataBuffer + MAX_SIZE, (dataAddress + iterations - 1)*blockSize, blockSize, 1, 0, 0);
    mfc_read_tag_status_all();
  }

  //If the data's size is not a multiple of 4096
  if (iterations*size != MAX_SIZE)
  {
    int remainder = size%MAX_SIZE;
    printf("Data size is not a multiple of 4096, doing final segment\n");
    mfc_get(dataBuffer, dataAddress + size - remainder, remainder, 0, 0, 0);

    for (int j = 0; j < remainder; ++j)
    {
      if (dataBuffer[j] > highThreshold)
      {
        dataBuffer[j] = strongEdgeValue;
      }
      else if (dataBuffer[j] < lowThreshold)
      {
        dataBuffer[j] = 0;
      }
      else
      {
        dataBuffer[j] = weakEdgeValue;
      }
    }

    mfc_put(dataBuffer, dataAddress + size - remainder, remainder, 0, 0, 0);
  }

  return 0;
}

