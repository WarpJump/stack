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
//define_stack(int);

define_stack(char);


int main(){
    size_t begin  = clock();
    dynamic_array(char) arr;
    StackCtor(&arr);
    for(size_t i = 0; i < kNumOfTests; ++i){
        push(&arr, (i % 257 + '0') % kCharLength);
    }
    //*arr.mem_begin = 1;
    push(&arr, 1);
    for(size_t i = 0; i < kNumOfTests; ++i){
        pop(&arr);
    }
    printf("%zu %zu\n", arr.size, arr.capacity);
    printf("Ok %zu\n", (clock() - begin) * kMillisInSecond / CLOCKS_PER_SEC);
}