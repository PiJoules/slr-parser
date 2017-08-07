# Language Features 

- Dependent types
  - Types parameterized by other types and values of other types
  - These are declared using square brackets [] when stating a type 
- Function overloading


## Types 

- Any
  - Implemented as `void*`
  - Requires casting before usage
- NoneType


## Syntax

```
# Tuple literal
x = {1, 2, 3}  # Creates a tuple of size 3 containing ints 
x = {1, 2.0, "3"}  # Creates a tuple of size 3 whose first element is an int, second is a double, and last is a string 


# Type notations 
# Notice how creating types is similar to calling functions

# Arrays
# - static sized containers whose contained type and size are known at compile time 
# - contain only 1 type 
# - Accessed via idexing with '[...]' by unsigned integrals
type[unsigned integral]  # Syntatic sugar for the next line
array[type, unsigned integral]  # Create an array containing type of certain size. The default value is whatever is specified by the default constructor.
array[unsigned literal]  # Type defaults to Any

# Lists
# - dynamic sized containers whose contained type is known at compile time 
# - contain only 1 type 
# - size may change at runtime
# - Accessed same as array 
list[type, unsigned literal]  # Create a list with an initial size
list[type]  # Create a list with initial size of 0 initially
list[unsigned literal]  # Create a list of Any type of specific size 
list[]  # Create list of Any type of size 0 initially 

# Tuples 
# - static sized container whose content types and size are known at compile time 
# - may contain different types
# - size does not change at runtime 
# - accessed same as array and list 
# - empty tuples are illegal
tuple[type, type, ...]
tuple[]  # Not allowed

# Structs 
# - same as tuple, but contents are accessed through members with '.'
{member1: type, member2: type, ...}  # Syntactic sugar for the next line
struct[member1=type, member2=type, ...]
{}  # Not allowed


# Declaring types
tdef typename type
tdef S {a: int, b: int}
tdef S {a: S, b: int}  # Self referential types are allowed 


# Array of strings of size 10
x: str[n]


# Struct types 
x: {a: str, b: int, c: char}  # x is a struct with fields a, b, and c mapping to a str, int, and char respectively


# Regular free function
def func(arg1, arg2, ...):
    # func body 
    ... 


# Classes 
class A:
    x_: int = 2

    def func() -> int:
        return x_

# Objects and methods are implemented as structs with functions that call them 
# The above example essentially compiles to 

tdef A {
    x_: int, 
    func: (A) -> int,
}

# Some mangled naming scheme I haven't thought of yet
def func_A_args__returns_int(self: A) -> int:
    return A->x_;

# So calls like this 
a.func()

# are compiled to this
func_A_args__returns_int(a)  # which eventually gets compiled to C 


# Creating classes with generic + dependent types  
# The synatx for classes are 
class ClassName(parent1, parent2, ...)[arg1, arg2, ...]:
    # class body 
    ...

# The array class can be thought of as 
class array[type T, val size]:
    def __index__(i: uint where i <= size) -> T:
        ...
```


## Examples


### Hello World

```
def main(arc: int, argv: str[]) -> int:
    print("Hello world")
    return 0
```
