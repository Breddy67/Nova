🚀 Nova Programming Language
<div align="center">
A complete, Python-inspired programming language with bytecode VM, 6 data structures, and full control flow - built in C++

https://img.shields.io/badge/C++-17-blue.svg
https://img.shields.io/badge/license-MIT-blue.svg
https://img.shields.io/badge/build-passing-brightgreen.svg
https://img.shields.io/badge/version-2.1-orange.svg
https://img.shields.io/badge/lines-2500%252B-blue.svg

</div>
📖 Table of Contents
About Nova

Features

Quick Start

Language Syntax

Data Structures

Functions & Recursion

Control Flow

Error Handling

String Operations

Examples

Architecture

Installation

Testing

License

📌 About Nova
Nova is a modern, statically-typed programming language with dynamic typing capabilities, built from scratch in C++17. It features a custom lexer, parser, Abstract Syntax Tree (AST), bytecode compiler, and a stack-based virtual machine.

Why Nova?
Python-inspired syntax - Clean and readable

Full control flow - If/else, loops, break, continue

6 built-in data structures - Bunch, Map, Stack, Queue, Heap, LinkedList

Function system - First-class functions with recursion support

Error handling - Try/Catch/Throw exception system

Slicing syntax - Powerful list/string slicing [start:end:step]

Static & Dynamic typing - Best of both worlds

REPL environment - Interactive development

Bytecode debugging - Inspect instruction execution

✨ Features
Feature	Status	Description
Lexer	✅	Tokenizes source code
Parser	✅	Builds Abstract Syntax Tree
Bytecode Compiler	✅	Compiles AST to bytecode
Virtual Machine	✅	Executes bytecode
Variables	✅	Dynamic (var) and Static (number, string, etc.)
Arithmetic	✅	+, -, *, /, %, **
Comparisons	✅	==, !=, <, >, <=, >=
Logical Operators	✅	and, or, not
If/Else	✅	Full branching with chaining
Loops	✅	loop (for-style) and while
Break/Continue	✅	Loop control flow
Functions	✅	First-class with recursion
Recursion	✅	Factorial, Sum_to working
Bunch (List)	✅	Dynamic array with slicing
Map (Dictionary)	✅	Key-value store
Stack (LIFO)	✅	Push, pop, top operations
Queue (FIFO)	✅	Enqueue, dequeue, front operations
Heap (Min-Heap)	✅	Priority queue operations
LinkedList	✅	Doubly linked list operations
Strings	✅	Mutable strings with slicing
Slicing	✅	[start:end:step] syntax
Error Handling	✅	Try/Catch/Throw
REPL	✅	Interactive development
Bytecode Debug	✅	Instruction-level debugging
🚀 Quick Start
Installation
bash
# Clone the repository
git clone https://github.com/Breddy67/Nova.git
cd Nova

# Compile with g++
g++ -std=c++17 -I. -o nova_new.exe src/Lexer.cpp src/Parser.cpp src/bytecode/BytecodeCompiler.cpp src/bytecode/NovaVM.cpp src/main.cpp
Run a Program
bash
# Run a Nova file
./nova_new.exe Tests/EX23.nv

# Start the REPL
./nova_new.exe
REPL Commands
Command	Purpose
exit / quit	Exit the REPL
debug	Toggle debug mode
📝 Language Syntax
Comments
text
# This is a single-line comment
Variables
nova
# Dynamic typing
var x = 10
var name = "Nova"
var flag = true
var nothing = null

# Static typing
number x = 10
string name = "Nova"
bool flag = true
null empty = null

# Assignment
x = 20
name = "Python"
Arithmetic
nova
var sum = 10 + 5
var diff = 20 - 3
var product = 4 * 6
var quotient = 15 / 3
var remainder = 10 % 3
var power = 2 ** 8
Comparisons
nova
print(10 == 10)   # true
print(10 != 5)    # true
print(10 < 20)    # true
print(10 > 20)    # false
Logical Operators
nova
print(true and true)    # true
print(true or false)    # true
print(not true)         # false
Print Statement
nova
# Basic print
print("Hello, World!")
print(42)

# Multiple arguments
print("Numbers:", 10, 20, 30)

# Custom separator
print(10, 20, 30, sep=", ")

# Custom ending (default is newline)
print("No newline", end="")
print("Another line", end="!!!")

# Debug print
debug("Variable x =", x)
📚 Data Structures
Bunch (Dynamic Array/List)
nova
# Creation
var list = bunch(1, 2, 3, 4, 5)
var empty = bunch()
var list2 = {1, 2, 3, 4}    # Sugar syntax

# Methods
bunch_push(list, 6)         # Add to end
var last = bunch_pop(list)  # Remove from end
var first = bunch_get(list, 0)  # Get element
var size = bunch_size(list) # Get size
var empty = bunch_empty(list)   # Check if empty

# Index access
var first = list[0]

# Slicing [start:end:step]
bunch b = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
b[0:5]      # [0, 1, 2, 3, 4]
b[3:7]      # [3, 4, 5, 6]
b[:5]       # [0, 1, 2, 3, 4]
b[5:]       # [5, 6, 7, 8, 9]
b[::2]      # [0, 2, 4, 6, 8]
Map (Dictionary)
nova
# Creation
var person = map()

# Methods
map_set(person, "name", "Alice")
map_set(person, "age", 25)

var name = map_get(person, "name")
var hasAge = map_has(person, "age")
var size = map_size(person)
var keys = map_keys(person)
Stack (LIFO)
nova
# Creation
var stack = stack()

# Methods
stack_push(stack, 10)
stack_push(stack, 20)
var top = stack_pop(stack)   # 20
var peek = stack_top(stack)  # 10
var size = stack_size(stack)
var empty = stack_empty(stack)
Queue (FIFO)
nova
# Creation
var queue = queue()

# Methods
queue_enqueue(queue, 10)
queue_enqueue(queue, 20)
var first = queue_dequeue(queue)   # 10
var front = queue_front(queue)     # 20
var size = queue_size(queue)
var empty = queue_empty(queue)
Heap (Min-Heap Priority Queue)
nova
# Creation
var heap = heap()

# Methods
heap_push(heap, 10)
heap_push(heap, 5)
heap_push(heap, 20)
var min = heap_pop(heap)    # 5
var top = heap_top(heap)    # 10
var size = heap_size(heap)
var empty = heap_empty(heap)
LinkedList (Doubly Linked List)
nova
# Creation
var list = linkedlist()

# Methods
ll_push_front(list, 10)
ll_push_back(list, 20)
var first = ll_pop_front(list)  # 10
var last = ll_pop_back(list)    # 20
var elem = ll_get(list, 0)
var size = ll_size(list)
var empty = ll_empty(list)
📦 Functions & Recursion
Function Declaration
nova
# Simple function
fun add(a, b) {
    return a + b
}

# Function with no return
fun greet(name) {
    print("Hello,", name)
}

# Recursive function - Factorial
fun factorial(n) {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

# Recursive function - Sum to n
fun sum_to(n) {
    if (n <= 0) {
        return 0
    }
    return n + sum_to(n - 1)
}

# Calling functions
var result = add(5, 3)      # 8
greet("Nova")               # Hello, Nova
print(factorial(5))         # 120
print(sum_to(5))            # 15
🔄 Control Flow
If/Else
nova
if (x > 10) {
    print("x is big")
} else if (x > 5) {
    print("x is medium")
} else {
    print("x is small")
}
Loops
nova
# For loop (C-style)
loop (i = 0; i < 10; i = i + 1) {
    print(i)
}

# While loop
var x = 0
while (x < 5) {
    print(x)
    x = x + 1
}

# Break and Continue
loop (i = 0; i < 10; i = i + 1) {
    if (i == 3) {
        continue    # Skip 3
    }
    if (i == 7) {
        break       # Stop at 7
    }
    print(i)
}
❌ Error Handling
Try/Catch/Throw
nova
# Basic try/catch
try {
    throw("Something went wrong!")
} catch (err) {
    print("Error:", err)
}

# Practical example
fun divide(a, b) {
    if (b == 0) {
        throw("Cannot divide by zero!")
    }
    return a / b
}

try {
    var result = divide(10, 0)
    print("Result:", result)
} catch (err) {
    print("Caught error:", err)
}

# Nested try/catch
try {
    print("Outer try")
    try {
        print("  Inner try")
        throw("Inner error")
    } catch (err) {
        print("  Inner caught:", err)
    }
    print("Back to outer try")
} catch (err) {
    print("Outer caught:", err)
}
📝 String Operations
nova
# Creation
var text = "Hello, Nova!"

# Methods
var len = string_size(text)          # Get length
var char = string_get(text, 0)       # Get character
string_set(text, 0, "J")             # Set character

# Slicing
print(text[0:5])      # Hello
print(text[7:12])     # Nova!
print(text[::2])      # Hlo oa
💡 Examples
Hello World
nova
print("Hello, World!")
Factorial with Recursion
nova
fun factorial(n) {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}
print(factorial(5))  # 120
Sum to N with Recursion
nova
fun sum_to(n) {
    if (n <= 0) {
        return 0
    }
    return n + sum_to(n - 1)
}
print(sum_to(5))  # 15
Complete Example
nova
print("=== Nova Programming Language ===")

# Variables
var x = 10
string name = "Nova"
print("x =", x)
print("name =", name)

# Arithmetic
var sum = x + 5
var product = sum * 2
print("sum =", sum)
print("product =", product)

# If/Else
if (x > 5) {
    print("x is big")
} else {
    print("x is small")
}

# Loop
loop (i = 0; i < 5; i = i + 1) {
    print(i)
}

# Functions
fun add(a, b) {
    return a + b
}
print("add(5, 3) =", add(5, 3))

# Recursion
fun factorial(n) {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}
print("factorial(5) =", factorial(5))

# Bunch (List)
var list = {1, 2, 3, 4, 5}
bunch_push(list, 6)
print("list =", list)
print("list[0] =", list[0])
print("list[0:3] =", list[0:3])

# Map (Dictionary)
var user = map()
map_set(user, "name", "Alice")
map_set(user, "age", 25)
print("name =", map_get(user, "name"))
print("keys =", map_keys(user))

# Stack
var stack = stack()
stack_push(stack, 10)
stack_push(stack, 20)
print("stack_pop =", stack_pop(stack))

# Queue
var queue = queue()
queue_enqueue(queue, "first")
queue_enqueue(queue, "second")
print("queue_dequeue =", queue_dequeue(queue))

# String Operations
var text = "Hello, Nova!"
print("text =", text)
print("text[0:5] =", text[0:5])
string_set(text, 0, "J")
print("modified =", text)

# Error Handling
try {
    throw("This is a test error")
} catch (err) {
    print("Caught:", err)
}
🏗️ Architecture
How Nova Works
text
Source Code → Lexer → Tokens → Parser → AST → Bytecode Compiler → Bytecode → VM → Output
Components
Component	File	Description
Lexer	Lexer.cpp/h	Converts source code to tokens
Parser	Parser.cpp/h	Converts tokens to AST
AST	AST.h	Defines AST nodes and data structures
Compiler	BytecodeCompiler.cpp/h	Converts AST to bytecode
Program	BytecodeProgram.h	Stores bytecode and constants
OpCodes	OpCodes.h	Defines all opcodes
VM	NovaVM.cpp/h	Executes bytecode
Environment	Environment.h	Manages variables and scoping
Main	main.cpp	Entry point, REPL, file execution
Opcodes
Opcode	Value	Description
PUSH	0	Push constant to stack
POP	1	Pop from stack
ADD	2	Addition
SUB	3	Subtraction
MUL	4	Multiplication
DIV	5	Division
MOD	6	Modulo
POW	7	Power
NEGATE	8	Negate number
EQ	9	Equal
NEQ	10	Not equal
LT	11	Less than
GT	12	Greater than
LTE	13	Less or equal
GTE	14	Greater or equal
AND	15	Logical AND
OR	16	Logical OR
NOT	17	Logical NOT
LOAD	18	Load variable
STORE	19	Store variable
DEFINE	20	Define variable
JUMP	21	Unconditional jump
JUMP_IF	22	Jump if true
JUMP_IF_NOT	23	Jump if false
CALL	24	Call function
RETURN	25	Return from function
PRINT	66	Print
HALT	71	Stop execution
📚 Keywords Summary
Keyword	Purpose
var	Dynamic variable declaration
number	Number type declaration
string	String type declaration
bool	Boolean type declaration
null	Null type declaration
bunch	List type declaration
map	Dictionary type declaration
stack	Stack type declaration
queue	Queue type declaration
heap	Heap type declaration
linkedlist	LinkedList type declaration
fun	Function declaration
return	Return from function
if	If condition
else	Else condition
loop	For loop
while	While loop
break	Break out of loop
continue	Skip to next iteration
print	Print to console
debug	Debug print
try	Start try block
catch	Catch error
throw	Throw error
true	Boolean true
false	Boolean false
null	Null value
and	Logical AND
or	Logical OR
not	Logical NOT
🔧 Installation
Prerequisites
C++17 compiler (g++ 7.0+)

Git (optional)

Build from Source
bash
# Clone the repository
git clone https://github.com/Breddy67/Nova.git
cd Nova

# Compile
g++ -std=c++17 -I. -o nova_new.exe src/Lexer.cpp src/Parser.cpp src/bytecode/BytecodeCompiler.cpp src/bytecode/NovaVM.cpp src/main.cpp

# Run
./nova_new.exe
Run Tests
bash
# Run a test file
./nova_new.exe Tests/EX23.nv

# Run all tests
for file in Tests/*.nv; do
    echo "Running $file..."
    ./nova_new.exe "$file"
done
🧪 Testing
Test Files
File	Test Case
EX1.nv	Basic Print
EX2.nv	Arithmetic
EX3.nv	Comparisons
EX4.nv	Logical Operators
EX5.nv	Variables
EX6.nv	Bunch Operations
EX7.nv	Bunch Loop
EX8.nv	Bunch Slicing
EX9.nv	Map Operations
EX10.nv	Stack Operations
EX11.nv	Map Loop
EX12.nv	Queue Operations
EX13.nv	Heap Operations
EX14.nv	LinkedList Operations
EX15.nv	String Operations
EX16.nv	If/Else
EX17.nv	String Slicing
EX18.nv	Nested If
EX19.nv	Functions
EX20.nv	Recursion (Factorial, Sum_to)
EX21.nv	While Loop
EX22.nv	Break/Continue
EX23.nv	Fibonacci Test
EX24.nv	Try/Catch/Throw
📋 Future Enhancements
Fix Fibonacci recursion bug

Classes and Object-Oriented Programming

File I/O operations

Modules and imports

List comprehensions

Lambda functions

Type inference

JIT compilation

Language Server Protocol (LSP) support

📄 License
This project is licensed under the MIT License - see the LICENSE file for details.

🙏 Acknowledgments
Inspired by Python's clean syntax

<<<<<<< HEAD
Built with C++17 for performance and systems programming
=======
Built with C++17 for performance and systems programming
>>>>>>> 7160908ce7fab4827520005247ba7f42c632da04
