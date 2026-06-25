# 🚀 Nova Programming Language

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

> **Nova** is a Python-inspired programming language with 8 built-in data structures and a bytecode VM. Built from scratch in C++17.

## ✨ Features

- **8 Built-in Data Structures**: Bunch (vector), Func (map), Stack, Queue, Heap, LinkedList, Tree, Graph
- **Python-Inspired Syntax**: Clean, readable, no semicolons
- **Bytecode VM**: 4x faster than AST interpretation
- **Full Language**: Functions, closures, loops, error handling
- **Slicing**: Python-style `b[0:5]` for arrays/strings
- **REPL**: Interactive development environment

## 🚀 Quick Start

```bash
git clone https://github.com/YOUR_USERNAME/nova-language.git
cd nova-language
mkdir build && cd build
cmake ..
make
./nova                    # REPL mode
./nova examples/demo.nv   # Run a file