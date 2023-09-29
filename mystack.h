#define concatenate(a, b) a##_##b
#define dynamic_array(type) concatenate(DynamicArr, type)

// TODO make extern global static structure for exceptions which will unroll all
// calls and fall until it catched
// TODO how to support in it as many different exceptions as needed?

static struct ExceptionControl {
#ifdef DEBUG
  size_t stack_exceptions;
#endif
} exceptions;

enum StackCodes {
  StackDoesNotExist = 1,
  NullArrOfNonemptyStack = 2,
  StackSizeNegative = 4,
  StackCapacityNegative = 8,
  StackCapacityLessSize = 16,
  CanaryFail = 32
};

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
    IF_CANARY(unsigned long long *last_canary;) \
    size_t capacity;                            \
    size_t size;                                \
  }

#ifdef DEBUG
#define STACK_DUMP(stack) StackDump(stack, __func__, __LINE__, __FILE__);
#define STACK_VERIFY(stack) Stack_Verify(stack)
#define THROW_STACK(code) exceptions.stack_exceptions |= code;

#define define_verificator(type)                                               \
  inline int Stack_Verify(dynamic_array(type) * stack) {                       \
    int return_value = 0;                                                      \
    if (!stack) {                                                              \
      THROW_STACK(StackCodes::StackDoesNotExist);                              \
      return 1;                                                                \
    }                                                                          \
    if (!stack->arr_begin && stack->size != 0) {                               \
      THROW_STACK(StackCodes::NullArrOfNonemptyStack);                         \
      return 1;                                                                \
    }                                                                          \
    if (stack->capacity < stack->size) {                                       \
      THROW_STACK(StackCodes::StackCapacityLessSize);                          \
      return_value = 1;                                                        \
    }                                                                          \
    IF_CANARY(if (stack->mem_begin &&                                         \
                  *reinterpret_cast<unsigned long long *>(stack->mem_begin) != \
                      kCanary) {                                               \
      THROW_STACK(StackCodes::CanaryFail);                                     \
      return_value = 1;                                                        \
    };)                                                                        \
    return return_value;                                                       \
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
    fprintf(stderr, "{\n size = %zu;\n capacity = %zu;\n data [%p] \n {\n", \
            stack->size, stack->capacity, stack->arr_begin);                \
    size_t log_size = stack->size > 10 ? 10 : stack->size;                  \
    for (size_t i = 0; i < log_size; ++i) {                                 \
      fprintf(stderr, "  [%d] = %d\n", i, *(stack->arr_begin + i));         \
    }                                                                       \
    if (stack->size > log_size) {                                           \
      fprintf(stderr, "  ...\n");                                           \
    }                                                                       \
    fprintf(stderr, " \n\n");                                               \
    if (exceptions.stack_exceptions & StackCodes::StackCapacityLessSize) {  \
      fprintf(stderr, "stack size is less that capacity\n");                \
    }                                                                       \
    abort();                                                                \
  }
#else
#define STACK_DUMP(stack)
#define STACK_VERIFY(stack)
#define THROW_STACK(code)
#define define_verificator(type)
#define define_dump(type)
#endif

#define define_resize(type)                                                    \
  inline int resize(dynamic_array(type) * stack, size_t capacity) {            \
    if ((stack == nullptr) || (capacity == 0)) {                               \
      return 1;                                                                \
    }                                                                          \
    size_t true_size = capacity * sizeof(type);                                \
                                                                               \
    /* shift needed to align array elements from first  Canary */              \
    size_t array_begin_shift = 0;                                              \
    /* shift needed to align second Canary from elements*/                     \
    size_t canary_2nd_shift = 0;                                               \
    IF_CANARY(array_begin_shift = ((sizeof(kCanary) - 1) / sizeof(type) + 1) * \
                                  sizeof(kCanary);)                            \
    IF_CANARY(canary_2nd_shift =                                               \
                  ((true_size - 1) / sizeof(kCanary) + 1) * sizeof(kCanary) -  \
                  true_size;)                                                  \
                                                                               \
    IF_CANARY(true_size +=                                                     \
              array_begin_shift + canary_2nd_shift + sizeof(kCanary);)         \
    stack->mem_begin =                                                         \
        reinterpret_cast<char *>(realloc(stack->mem_begin, true_size));        \
    stack->arr_begin =                                                         \
        reinterpret_cast<type *>(stack->mem_begin + array_begin_shift);        \
    IF_CANARY(stack->last_canary = reinterpret_cast<unsigned long long *>(     \
                  stack->mem_begin + true_size - sizeof(kCanary));)            \
    IF_CANARY(*reinterpret_cast<long long *>(stack->mem_begin) = kCanary;)     \
    IF_CANARY(*reinterpret_cast<long long *>(stack->last_canary) = kCanary;)   \
                                                                               \
    stack->capacity = capacity;                                                \
    return 0;                                                                  \
  }

#define define_stack_constructor(type)                           \
  void StackCtor(dynamic_array(type) * stack) {                  \
    stack->arr_begin = nullptr;                                  \
    stack->mem_begin = nullptr;                                  \
    IF_CANARY(stack->last_canary = nullptr;)                     \
    stack->size = 0;                                             \
    stack->capacity = 0;                                         \
  }                                                              \
                                                                 \
  void StackCtor(dynamic_array(type) * stack, size_t capacity) { \
    resize(stack, capacity);                                     \
    stack->size = 0;                                             \
  }

#define define_stack_destructor(type)           \
  void StackDtor(dynamic_array(type) * stack) { \
    if (stack->mem_begin != nullptr) {          \
      free(stack->mem_begin);                   \
    }                                           \
    stack->capacity = 0;                        \
    stack->size = 0;                            \
  }

#define define_push(type)                           \
  int push(dynamic_array(type) * stack, type obj) { \
    STACK_VERIFY(stack);                            \
    STACK_DUMP(stack);                              \
    if (stack->size >= stack->capacity) {           \
      if (stack->capacity == 0) {                   \
        resize(stack, 2);                           \
      } else {                                      \
        resize(stack, 2 * stack->capacity);         \
      }                                             \
    }                                               \
    (stack->arr_begin)[stack->size] = obj;          \
    ++stack->size;                                  \
    return 0;                                       \
  }

#define define_pop(type)                     \
  int pop(dynamic_array(type) * stack) {     \
    STACK_VERIFY(stack);                     \
    STACK_DUMP(stack);                       \
    if (stack->size * 3 < stack->capacity) { \
      resize(stack, stack->capacity / 2);    \
    }                                        \
    if (stack->size > 0) {                   \
      --stack->size;                         \
      return 0;                              \
    }                                        \
    return 1;                                \
  }

#define define_peek(type)                      \
  type *peek(dynamic_array(type) * stack) {    \
    if (stack->size > 0) {                     \
      return get_elem(stack, stack->size - 1); \
    }                                          \
    return nullptr;                            \
  }

#define define_get(type)                                      \
  type *get_elem(dynamic_array(type) * stack, size_t index) { \
    return (stack->arr_begin + index);                        \
  }

#define define_print(type)                      \
  void print_vec(dynamic_array(type) * stack) { \
    for (int i = 0; i < stack->capacity; ++i) { \
      printf("%d ", *(stack->arr_begin + i));   \
    }                                           \
    putchar('\n');                              \
  }

#define define_stack(type)        \
  define_dynamic_array(type);     \
  define_verificator(type);       \
  define_dump(type);              \
  define_resize(type);            \
  define_stack_constructor(type); \
  define_stack_destructor(type);  \
  define_get(type);               \
  define_peek(type);              \
  define_pop(type);               \
  define_push(type);              \
  define_print(type);
