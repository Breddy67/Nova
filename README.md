# 🚀 Nova Programming Language

<div align="center">

**A complete, Python-inspired programming language with bytecode VM, 6 data structures, and full control flow - built in C++**

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/Breddy67/Nova)
[![Version](https://img.shields.io/badge/version-2.1-orange.svg)](https://github.com/Breddy67/Nova)
[![Lines](https://img.shields.io/badge/lines-2500%2B-blue.svg)](https://github.com/Breddy67/Nova)

</div>

---

## 📖 Table of Contents

- [About Nova](#-about-nova)
- [Features](#-features)
- [Quick Start](#-quick-start)
- [Language Syntax](#-language-syntax)
- [Data Structures](#-data-structures)
- [Functions & Recursion](#-functions--recursion)
- [Control Flow](#-control-flow)
- [Error Handling](#-error-handling)
- [String Operations](#-string-operations)
- [Examples](#-examples)
- [Architecture](#-architecture)
- [Installation](#-installation)
- [Testing](#-testing)
- [License](#-license)

---

## 📌 About Nova

**Nova** is a modern programming language with dynamic and static typing capabilities, built from scratch in C++17. It features a custom lexer, parser, Abstract Syntax Tree (AST), bytecode compiler, and a stack-based virtual machine.

### Why Nova?

- **Python-inspired syntax** - Clean and readable
- **Full control flow** - If/else, loops, break, continue
- **6 built-in data structures** - Bunch, Map, Stack, Queue, Heap, LinkedList
- **Function system** - First-class functions with recursion support
- **Error handling** - Try/Catch/Throw exception system
- **Slicing syntax** - Powerful list/string slicing `[start:end:step]`
- **Static & Dynamic typing** - Best of both worlds
- **REPL environment** - Interactive development
- **Bytecode debugging** - Inspect instruction execution

---

## ✨ Features

| Feature | Status | Description |
|---------|--------|-------------|
| **Lexer** | ✅ | Tokenizes source code |
| **Parser** | ✅ | Builds Abstract Syntax Tree |
| **Bytecode Compiler** | ✅ | Compiles AST to bytecode |
| **Virtual Machine** | ✅ | Executes bytecode |
| **Variables** | ✅ | Dynamic (`var`) and Static (`number`, `string`, etc.) |
| **Arithmetic** | ✅ | `+`, `-`, `*`, `/`, `%`, `**` |
| **Comparisons** | ✅ | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| **Logical Operators** | ✅ | `and`, `or`, `not` |
| **If/Else** | ✅ | Full branching with chaining |
| **Loops** | ✅ | `loop` (for-style) and `while` |
| **Break/Continue** | ✅ | Loop control flow |
| **Functions** | ✅ | First-class with recursion |
| **Recursion** | ✅ | Factorial, Sum_to working |
| **Bunch (List)** | ✅ | Dynamic array with slicing |
| **Map (Dictionary)** | ✅ | Key-value store |
| **Stack (LIFO)** | ✅ | Push, pop, top operations |
| **Queue (FIFO)** | ✅ | Enqueue, dequeue, front operations |
| **Heap (Min-Heap)** | ✅ | Priority queue operations |
| **LinkedList** | ✅ | Doubly linked list operations |
| **Strings** | ✅ | Mutable strings with slicing |
| **Slicing** | ✅ | `[start:end:step]` syntax |
| **Error Handling** | ✅ | Try/Catch/Throw |
| **REPL** | ✅ | Interactive development |
| **Bytecode Debug** | ✅ | Instruction-level debugging |

---

## 🚀 Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/Breddy67/Nova.git
cd Nova

# Compile with g++
g++ -std=c++17 -I. -o nova_new.exe src/Lexer.cpp src/Parser.cpp src/bytecode/BytecodeCompiler.cpp src/bytecode/NovaVM.cpp src/main.cpp
