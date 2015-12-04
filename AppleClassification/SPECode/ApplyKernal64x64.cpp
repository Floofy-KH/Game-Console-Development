#include <spu_mfcio.h>

#define TAG 1
#define WIDTH 64
#define HEIGHT 64
#define BUF_SIZE WIDTH*HEIGHT

int main(unsigned long long arg1, unsigned long long arg2, unsigned long long arg3)
{
  int inDataBuffer[BUF_SIZE] __attribute__((aligned(128))); 
  int kernalBuffer[9] __attribute__((aligned(32)));
  int outDataBuffer[BUF_SIZE] __attribute__((aligned(128)));

  mfc_get(inDataBuffer, arg1, sizeof(inDataBuffer), 0, 0, 0);
  mfc_get(kernalBuffer, arg1, sizeof(kernalBuffer), 0, 0, 0);
}