#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <cassert>
#define DEBUG
#define HASH
#define CANARY

#include "mystack.h"

const long long kNumOfTests = 10'000;
const int kMillisInSecond = 1000;
const int kCharLength = 256;

typedef char* CharPtr;
// define_stack(int);

define_stack(char);

int main() {
  size_t begin = clock();
  dynamic_array(char) arr;
  StackCtor(&arr);
  for (size_t i = 0; i < kNumOfTests; ++i) {
    Push(&arr, (i % 257 + '0') % kCharLength);
  }
  //Push(&arr, 1);
  printf("%zu\n", *Peek(&arr));

  //uncomment to see hash protection fail
  // arr.arr_begin[0] = 13; 
  
  //uncomment to see both hash and canary protection fail
  // arr.mem_begin[0] = 24; 
  for (size_t i = 0; i < kNumOfTests; ++i) {
    Pop(&arr);
  }
  printf("Final size %zu, final capacity %zu\n", arr.size, arr.capacity);
  printf("Ok %zu\n", (clock() - begin) * kMillisInSecond / CLOCKS_PER_SEC);
  StackDtor(&arr);
}