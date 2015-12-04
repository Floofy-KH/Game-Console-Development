#include <spu_mfcio.h>
#include <stdio.h>

struct stuff
{
  int numbers[31];
  int sum;
};

int main(unsigned long long spe_id, unsigned long long argp, unsigned long long envp)
{

  static stuff thing __attribute__((aligned(128)));
  mfc_get(&thing, argp, sizeof(thing), 0, 0, 0);

  thing.sum = 0;
  
  thing.sum += thing.numbers[0];
  thing.sum += thing.numbers[1];
  thing.sum += thing.numbers[2];

  for (int i = 3; i < 31; ++i)
  {
    printf("%l ", thing.sum);
    thing.sum += thing.numbers[i];
  }

  mfc_put(&thing, argp, sizeof(thing), 0, 0, 0);

  return 0;
}
