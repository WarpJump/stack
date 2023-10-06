#define concatenate(a, b) a##b
#define dynamic_array(type) concatenate(DynamicArr, type)

// TODO make extern global static structure for exceptions which will unroll all
// calls and fall until it catched
// TODO how to support in it as many different exceptions as needed?

static struct ExceptionControl {
#ifdef DEBUG
  size_t stack_exceptions = 0;
#else
#endif
} exceptions;

enum StackCodes {
  StackDoesNotExist = 1,
  NullArrOfNonemptyStack = 2,
  StackCapacityLessSize = 16,
  CanaryFail = 32,
  HashFail = 64
};

#ifdef DEBUG
#define IF_DEBUG(...) __VA_ARGS__
#else
#define IF_DEBUG(...)
#endif

#ifdef HASH
const long long kPrimeNumber = 12553;
const unsigned long long kModule = 1e9 + 7;

#define IF_HASH(...) __VA_ARGS__
#else
#define IF_HASH(...)
#endif

#ifdef CANARY
const long long kCanary = 0x77777777;
#define IF_CANARY(...) __VA_ARGS__
#else
#define IF_CANARY(...)
#define add_pointers
#endif

#define define_dynamic_array(type)              \
  struct dynamic_array(type) {                  \
    type *arr_begin;                            \
    char *mem_begin;                            \
    IF_HASH(size_t hash);                       \
    IF_CANARY(unsigned long long *last_canary;) \
    size_t mem_allocated;                       \
    size_t capacity;                            \
    size_t size;                                \
  }

#ifdef DEBUG
#define STACK_DUMP(stack) StackDump(stack, __func__, __LINE__, __FILE__);
#define STACK_VERIFY(stack) STACK_VERIFY(stack)
#define THROW_STACK(code) exceptions.stack_exceptions |= code;

#define define_verificator(type)                         \
  inline int STACK_VERIFY(dynamic_array(type) * stack) { \
    int return_value = 0;                                \
    if (!stack) {                                        \
      THROW_STACK(StackCodes::StackDoesNotExist);        \
      return 1;                                          \
    }                                                    \
    if (!stack->arr_begin && stack->size != 0) {         \
      THROW_STACK(StackCodes::NullArrOfNonemptyStack);   \
      return 1;                                          \
    }                                                    \
    if (stack->capacity < stack->size) {                 \
      THROW_STACK(StackCodes::StackCapacityLessSize);    \
      return_value = 1;                                  \
    }                                                    \
    IF_CANARY(if (!CanaryVerify(stack)) {                \
      THROW_STACK(StackCodes::CanaryFail);               \
      return_value = 1;                                  \
    })                                                   \
    IF_HASH(if (!HashVerify(stack)) {                    \
      THROW_STACK(StackCodes::HashFail);                 \
      return_value = 1;                                  \
    })                                                   \
    return return_value;                                 \
  }

#define define_dump(type)                                                   \
  inline void StackDump(dynamic_array(type) * stack, const char *function,  \
                        const size_t line, const char *filename) {          \
    if (!exceptions.stack_exceptions) {                                     \
      return;                                                               \
    }                                                                       \
    fprintf(stderr, "ERROR in Stack %p in function %s in file %s:%zu\n",    \
            stack, function, filename, line);                               \
    if (exceptions.stack_exceptions & StackCodes::StackDoesNotExist) {      \
      fprintf(stderr, "Stack is nullptr\n");                                \
      return;                                                               \
    }                                                                       \
    IF_CANARY(if (exceptions.stack_exceptions & StackCodes::CanaryFail) {   \
      fprintf(stderr, "<<<Canary protection failed>>>\n");                  \
    })                                                                      \
    IF_HASH(if (exceptions.stack_exceptions & StackCodes::HashFail) {       \
      fprintf(stderr, "<<<Hash protection failed>>>\n");                    \
    })                                                                      \
    fprintf(stderr, "{\n size = %zu;\n capacity = %zu;\n data [%p] \n {\n", \
            stack->size, stack->capacity, stack->arr_begin);                \
    size_t log_size = stack->size > 10 ? 10 : stack->size;                  \
    PrintVec(stack, stderr, log_size);                                      \
    if (stack->size > log_size) {                                           \
      fprintf(stderr, "  ...\n");                                           \
    }                                                                       \
    fprintf(stderr, " }\n}\n");                                             \
    if (exceptions.stack_exceptions & StackCodes::StackCapacityLessSize) {  \
      fprintf(stderr, "stack size is less that capacity\n");                \
    }                                                                       \
    fprintf(stderr, "Terminating the program\n");                           \
    abort();                                                                \
  }
#else
#define STACK_DUMP(stack)
#define STACK_VERIFY(stack)
#define THROW_STACK(code)
#define define_verificator(type)
#define define_dump(type)
#endif

#define define_canary(type)                                              \
  int CanaryVerify(dynamic_array(type) * stack) {                        \
    return !stack->mem_begin || *reinterpret_cast<unsigned long long *>( \
                                    stack->mem_begin) == kCanary;        \
  }

#define define_hashing(type)                                             \
  size_t HashStack(dynamic_array(type) * stack) {                        \
    stack->hash = 0;                                                     \
    size_t new_hash = 0;                                                 \
    for (size_t i = 0; i < sizeof(*stack); ++i) {                        \
      new_hash = new_hash * kPrimeNumber % kModule + ((char *)stack)[i]; \
      new_hash %= kModule;                                               \
    }                                                                    \
    for (size_t i = 0; i < stack->size; ++i) {                           \
      new_hash += ((char *)stack->mem_begin)[i];                         \
    }                                                                    \
    stack->hash = new_hash;                                              \
    return new_hash;                                                     \
  }                                                                      \
                                                                         \
  size_t HashVerify(dynamic_array(type) * stack) {                       \
    size_t old_hash = stack->hash;                                       \
    return HashStack(stack) == old_hash;                                 \
  }

#define define_resize(type)                                                    \
  inline int Resize(dynamic_array(type) * stack, size_t capacity) {            \
    if ((stack == nullptr) || (capacity == 0)) {                               \
      return 1;                                                                \
    }                                                                          \
    size_t mem_allocated = capacity * sizeof(type);                            \
    /* shift needed to align array elements from first  Canary */              \
    size_t array_begin_shift = 0;                                              \
    /* shift needed to align second Canary from elements*/                     \
    size_t canary_2nd_shift = 0;                                               \
    IF_CANARY(array_begin_shift = ((sizeof(kCanary) - 1) / sizeof(type) + 1) * \
                                  sizeof(kCanary);)                            \
    IF_CANARY(canary_2nd_shift = ((mem_allocated - 1) / sizeof(kCanary) + 1) * \
                                     sizeof(kCanary) -                         \
                                 mem_allocated;)                               \
                                                                               \
    IF_CANARY(mem_allocated +=                                                 \
              array_begin_shift + canary_2nd_shift + sizeof(kCanary);)         \
    stack->mem_begin =                                                         \
        reinterpret_cast<char *>(realloc(stack->mem_begin, mem_allocated));    \
    stack->arr_begin =                                                         \
        reinterpret_cast<type *>(stack->mem_begin + array_begin_shift);        \
    IF_CANARY(stack->last_canary = reinterpret_cast<unsigned long long *>(     \
                  stack->mem_begin + mem_allocated - sizeof(kCanary));)        \
    IF_CANARY(*reinterpret_cast<long long *>(stack->mem_begin) = kCanary;)     \
    IF_CANARY(*reinterpret_cast<long long *>(stack->last_canary) = kCanary;)   \
                                                                               \
    stack->mem_allocated = mem_allocated;                                      \
    stack->capacity = capacity;                                                \
    return 0;                                                                  \
  }

#define define_stack_constructor(type)                          \
  int StackCtor(dynamic_array(type) * stack) {                  \
    IF_DEBUG(if (!stack) { return 1; })                         \
    for (size_t i = 0; i < sizeof(*stack); ++i) {               \
      ((char *)stack)[i] = 0;                                   \
    }                                                           \
    stack->arr_begin = nullptr;                                 \
    stack->mem_begin = nullptr;                                 \
    IF_CANARY(stack->last_canary = nullptr;)                    \
    stack->size = 0;                                            \
    stack->capacity = 0;                                        \
    stack->mem_allocated = 0;                                   \
    IF_HASH(stack->hash = 0; HashStack(stack));                 \
    return 0;                                                   \
  }                                                             \
                                                                \
  int StackCtor(dynamic_array(type) * stack, size_t capacity) { \
    IF_DEBUG(if (!stack) { return 1; })                         \
    Resize(stack, capacity);                                    \
    stack->size = 0;                                            \
    return 0;                                                   \
  }

#define define_stack_destructor(type)           \
  void StackDtor(dynamic_array(type) * stack) { \
    assert(stack);                              \
    if (stack->mem_begin != nullptr) {          \
      free(stack->mem_begin);                   \
    }                                           \
    stack->capacity = 0;                        \
    stack->size = 0;                            \
  }

#define define_push(type)                           \
  int Push(dynamic_array(type) * stack, type obj) { \
    STACK_VERIFY(stack);                            \
    STACK_DUMP(stack);                              \
    if (stack->size >= stack->capacity) {           \
      if (stack->capacity == 0) {                   \
        Resize(stack, 2);                           \
      } else {                                      \
        Resize(stack, 2 * stack->capacity);         \
      }                                             \
    }                                               \
    (stack->arr_begin)[stack->size] = obj;          \
    ++stack->size;                                  \
    IF_HASH(HashStack(stack));                      \
    return 0;                                       \
  }

#define define_pop(type)                     \
  int Pop(dynamic_array(type) * stack) {     \
    STACK_VERIFY(stack);                     \
    STACK_DUMP(stack);                       \
    if (stack->size * 4 < stack->capacity) { \
      Resize(stack, stack->capacity >> 1);   \
    }                                        \
    if (stack->size > 0) {                   \
      --stack->size;                         \
      IF_HASH(HashStack(stack));             \
      return 0;                              \
    }                                        \
    IF_HASH(HashStack(stack));               \
    return 1;                                \
  }

#define define_peek(type)                     \
  type *Peek(dynamic_array(type) * stack) {   \
    STACK_VERIFY(stack);                      \
    STACK_DUMP(stack);                        \
    if (stack->size > 0) {                    \
      return GetElem(stack, stack->size - 1); \
    }                                         \
    return nullptr;                           \
  }

#define define_get(type)                                     \
  type *GetElem(dynamic_array(type) * stack, size_t index) { \
    return (stack->arr_begin + index);                       \
  }

#define define_print(type)                                         \
  void PrintVec(dynamic_array(type) * stack, FILE * file,          \
                ssize_t number = -1) {                             \
    if (number == -1) {                                            \
      number = stack->capacity;                                    \
    }                                                              \
    for (int i = 0; i < number; ++i) {                             \
      fprintf(file, "  [%d] ", i);                                 \
      char *elem = (char *)stack->arr_begin + i;                   \
      for (int j = sizeof(stack->arr_begin[0]) - 1; j >= 0; --j) { \
        char byte = elem[j];                                       \
        for (int runner = 7; runner >= 0; --runner) {              \
          putc(((byte >> runner) & 1) + '0', file);                \
        }                                                          \
        putc(' ', file);                                           \
      }                                                            \
      putc('\n', file);                                            \
    }                                                              \
  }

#define define_stack(type)        \
  define_dynamic_array(type);     \
  IF_HASH(define_hashing(type));  \
  define_resize(type);            \
  define_print(type);             \
  IF_CANARY(define_canary(type)); \
  define_verificator(type);       \
  define_dump(type);              \
  define_stack_constructor(type); \
  define_stack_destructor(type);  \
  define_get(type);               \
  define_peek(type);              \
  define_pop(type);               \
  define_push(type);
