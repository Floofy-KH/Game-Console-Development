#include <spu_mfcio.h>

#define MAX_SIZE 4096

int main(vector unsigned long long arg1,
vector unsigned long long arg2, 
vector unsigned long long arg3)
{
  int dataAddress = ((vector signed int)arg1)[0];
  int size = ((vector signed int)arg1)[1];
  int lowThreshold = ((vector signed int)arg1)[2];
  int highThreshold = ((vector signed int)arg1)[3];
  int strongEdgeValue = ((vector signed int)arg2)[0];
  int weakEdgeValue = ((vector signed int)arg2)[0];

  int dataBuffer[4096*2] __attribute__((aligned(128)));
  int blockSize = sizeof(dataBuffer) / 2;
  int start = 0, end = 0;

  int iterations = MAX_SIZE / size;

  if (iterations > 1)
  {
    mfc_get(dataBuffer, dataAddress, blockSize, 0, 0, 0);
    for (int i = 0; i < iterations; ++i)
    {
      mfc_get(dataBuffer + (i & 1)*MAX_SIZE, dataAddress + i*blockSize, blockSize, i&1, 0, 0);
      
      mfc_write_tag_mask(1 << (1 - (1 & 1)));
      mfc_read_tag_status_all();

      start = (1 & i) ? 0 : MAX_SIZE;
      end = start + MAX_SIZE;
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

      mfc_put(dataBuffer + (1 - (i & 1))*MAX_SIZE, dataAddress + (i - 1)*blockSize, blockSize, 1 - (i & 1), 0, 0);
    }

    mfc_write_tag_mask(2);
    mfc_read_tag_status_all();

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

    mfc_put(dataBuffer + MAX_SIZE, (dataAddress + iterations - 1)*blockSize, blockSize, 1, 0, 0);
    mfc_read_tag_status_all();
  }

  //If the data's size is not a multiple of 4096
  if (iterations*size != MAX_SIZE)
  {
    int remainder = size%MAX_SIZE;
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

