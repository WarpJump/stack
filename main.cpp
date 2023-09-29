#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define CANARY
#define DEBUG
#include "mystack.h"

typedef char* CharPtr;
//define_stack(int);

define_stack(char);



int main(){
    size_t begin  = clock();
    dynamic_array(char) arr;
    StackCtor(&arr);
    for(size_t i = 0; i < 10'000'000; ++i){
        push(&arr, (i % 257 + '0') % 256);
    }
    push(&arr, 1);
    for(size_t i = 0; i < 10'000'000; ++i){
        pop(&arr);
    }
    printf("%zu %zu\n", arr.size, arr.capacity);
    printf("Ok %zu\n", (clock() - begin) * 1000 / CLOCKS_PER_SEC);
}