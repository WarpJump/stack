# stack

# Stack declaration
to define stack with your type you have to write
```
define_stack(yourtype);
```
in global namespace
To declare a variable of stack type, use macro     
```
dynamic_array(yourtype) stackname;
```
# Stack usage
* StackCtor(&stack) - sets default variables to previously created stack. Need to be called before StackDtor(&stack).
* Push(&stack, variable) - pushes a variable into stack
* Pop(&stack) - deletes the last elemtn, return error code. 0 if OK
* Peek(&stack) - returns pointer to the last element
* StackDtor(%stack) - realizes all allocated memory and destroys stack. Previously must be called StackCtor on same stack.
