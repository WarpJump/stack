#define concatenate(a, b) a##_##b
#define dynamic_array(type) concatenate(DynamicArr, type)

// TODO make extern global static structure for exceptions which will unroll all
// calls and fall until it catched
// TODO how to support in it as many different exceptions as needed?

static struct exception_control {
#ifdef DEBUG
  size_t stack_exceptions;
#endif
} exceptions;

enum stack_codes {
  stack_does_not_exist = 1,
  null_arr_of_nonempty_stack = 2,
  stack_size_negative = 4,
  stack_capacity_negative = 8,
  stack_capacity_less_size = 16
};

#define define_dynamic_array(type) \
  struct dynamic_array(type) {     \
    type *arr;                     \
    size_t capacity;               \
    size_t size;                   \
  }

#ifdef DEBUG
#define STACK_DUMP(stack) StackDump(stack, __func__, __LINE__, __FILE__);
#define STACK_VERIFY(stack) Stack_Verify(stack)
#define THROW_STACK(code) exceptions.stack_exceptions |= code;

#define define_verificator(type)                            \
  inline int Stack_Verify(dynamic_array(type) * stack) {    \
    int return_value = 0;                                   \
    if (!stack) {                                           \
      THROW_STACK(stack_codes::stack_does_not_exist);       \
      return 1;                                             \
    }                                                       \
    if (!stack->arr && stack->size != 0) {                  \
      THROW_STACK(stack_codes::null_arr_of_nonempty_stack); \
      return 1;                                             \
    }                                                       \
    if (stack->size < 0) {                                  \
      THROW_STACK(stack_codes::stack_size_negative);        \
      return_value = 1;                                     \
    }                                                       \
    if (stack->capacity < 0) {                              \
      THROW_STACK(stack_codes::stack_capacity_negative);    \
      return_value = 1;                                     \
    }                                                       \
    if (stack->capacity < stack->size) {                    \
      THROW_STACK(stack_codes::stack_capacity_less_size);   \
      return_value = 1;                                     \
    }                                                       \
    return return_value;                                    \
  }

#define define_dump(type)                                                      \
  inline void StackDump(dynamic_array(type) * stack, const char *function,     \
                        const size_t line, const char *filename) {             \
    if (!exceptions.stack_exceptions) {                                        \
      return;                                                                  \
    }                                                                          \
    fprintf(stderr, "error in Stack %p in function %s in file %s:%zu\n",       \
            stack, function, filename, line);                                  \
    if (exceptions.stack_exceptions & stack_codes::stack_does_not_exist) {     \
      fprintf(stderr, "Stack is nullptr\n");                                   \
      return;                                                                  \
    }                                                                          \
    fprintf(stderr, "{\n size = %zu;\n capacity = %zu;\n data [%p] \n {\n",    \
            stack->size, stack->capacity, stack->arr);                         \
    size_t log_size = stack->size > 10 ? 10 : stack->size;                     \
    for (size_t i = 0; i < log_size; ++i) {                                    \
      printf("  [%d] = %d\n", i, *(stack->arr + i));                           \
    }                                                                          \
    if (stack->size > log_size) {                                              \
      printf(" ...\n");                                                         \
    }                                                                          \
    printf(" }\n}\n");                                                         \
    if (exceptions.stack_exceptions & stack_codes::stack_size_negative) {      \
      fprintf(stderr, "stack size is negative\n");                             \
    }                                                                          \
    if (exceptions.stack_exceptions & stack_codes::stack_capacity_negative) {  \
      fprintf(stderr, "stack capacity is negative\n");                         \
    }                                                                          \
    if (exceptions.stack_exceptions & stack_codes::stack_capacity_less_size) { \
      fprintf(stderr, "stack size is less that capacity\n");                   \
    }                                                                          \
    abort();                                                                   \
  }
#else
#define STACK_DUMP(stack)
#define STACK_VERIFY(stack)
#define THROW_STACK(code)
#define define_verificator(type)
#define define_dump(type)
#endif

#define define_resize(type)                                         \
  inline int resize(dynamic_array(type) * stack, size_t capacity) { \
    if ((stack == nullptr) || (capacity == 0)) {                    \
      return 1;                                                     \
    } else {                                                        \
      stack->arr = reinterpret_cast<type *>(                        \
          realloc(stack->arr, capacity * sizeof(type)));            \
    }                                                               \
    stack->capacity = capacity;                                     \
    return 0;                                                       \
  }

#define define_stack_constructor(type)                           \
  void StackCtor(dynamic_array(type) * stack) {                  \
    stack->arr = nullptr;                                        \
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
    if (stack->arr != nullptr) {                \
      free(stack->arr);                         \
    }                                           \
    stack->capacity = 0;                        \
    stack->size = 0;                            \
  }

#define define_push(type)                           \
  int push(dynamic_array(type) * stack, type obj) { \
    STACK_VERIFY(stack);                            \
    if (stack->size >= stack->capacity) {           \
      if (stack->capacity == 0) {                   \
        resize(stack, 2);                           \
      } else {                                      \
        resize(stack, 2 * stack->capacity);         \
      }                                             \
    }                                               \
    (stack->arr)[stack->size] = obj;                \
    ++stack->size;                                  \
    return 0;                                       \
  }

#define define_pop(type)                 \
  int pop(dynamic_array(type) * stack) { \
    STACK_VERIFY(stack);                 \
    STACK_DUMP(stack);                   \
    if (stack->size > 0) {               \
      --stack->size;                     \
      return 0;                          \
    }                                    \
    return 1;                            \
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
    return (stack->arr + index);                              \
  }

#define define_print(type)                      \
  void print_vec(dynamic_array(type) * stack) { \
    for (int i = 0; i < stack->capacity; ++i) { \
      printf("%d ", *(stack->arr + i));         \
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
