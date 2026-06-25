#pragma once
#include <cstdint>

enum class OpCode : uint8_t {
    // ── Stack Operations ──────────────────────────────────────────────
    PUSH,           // Push constant to stack
    POP,            // Pop from stack
    
    // ── Arithmetic ────────────────────────────────────────────────────
    ADD,            // a + b
    SUB,            // a - b
    MUL,            // a * b
    DIV,            // a / b
    MOD,            // a % b
    POW,            // a ** b
    NEGATE,         // -a
    
    // ── Comparison ───────────────────────────────────────────────────
    EQ,             // a == b
    NEQ,            // a != b
    LT,             // a < b
    GT,             // a > b
    LTE,            // a <= b
    GTE,            // a >= b
    
    // ── Logical ──────────────────────────────────────────────────────
    AND,            // a and b
    OR,             // a or b
    NOT,            // not a
    
    // ── Variables ────────────────────────────────────────────────────
    LOAD,           // Load variable
    STORE,          // Store variable
    DEFINE,         // Define new variable
    
    // ── Control Flow ─────────────────────────────────────────────────
    JUMP,           // Unconditional jump
    JUMP_IF,        // Jump if truthy
    JUMP_IF_NOT,    // Jump if falsy
    
    // ── Functions ────────────────────────────────────────────────────
    FUN_DECL,
    CALL,           // Call function
    RETURN,         // Return from function
    
    // ── Bunch (Vector) ──────────────────────────────────────────────
    BUNCH_INIT,     // Create empty bunch
    BUNCH_PUSH,     // bunch.push(value)
    BUNCH_POP,      // bunch.pop()
    BUNCH_GET,      // bunch[index]
    BUNCH_SIZE,     // bunch.size()
    BUNCH_EMPTY,    // bunch.empty()
    // ── String ─────────────────────────────────────────────────────────
    STRING_INIT,    // Create empty string
    STRING_SET,     // string = value
    STRING_GET,     // string[index]
    STRING_SIZE,    // string.size()
    SLICE,
    
    // ── Func (Map) ───────────────────────────────────────────────────
    MAP_INIT,      // Create empty func
    MAP_SET,       // func[key] = value
    MAP_GET,       // func[key]
    MAP_HAS,       // func.has(key)
    MAP_SIZE,      // func.size()
    MAP_KEYS,       //func.keys()
    // ── Stack ────────────────────────────────────────────────────────
    STACK_INIT,     // Create empty stack
    STACK_PUSH,     // stack.push(value)
    STACK_POP,      // stack.pop()
    STACK_TOP,      // stack.top()
    STACK_SIZE,     // stack.size()
    
    // ── Queue ────────────────────────────────────────────────────────
    QUEUE_INIT,     // Create empty queue
    QUEUE_ENQUEUE,  // queue.enqueue(value)
    QUEUE_DEQUEUE,  // queue.dequeue()
    QUEUE_FRONT,    // queue.front()
    QUEUE_SIZE,     // queue.size()
    
    // ── Heap ─────────────────────────────────────────────────────────
    HEAP_INIT,      // Create empty heap
    HEAP_PUSH,      // heap.push(value)
    HEAP_POP,       // heap.pop()
    HEAP_TOP,       // heap.top()
    HEAP_SIZE,      // heap.size()
    
    // ── LinkedList ───────────────────────────────────────────────────
    LIST_INIT,      // Create empty linkedlist
    LIST_PUSH_FRONT,// list.push_front(value)
    LIST_PUSH_BACK, // list.push_back(value)
    LIST_POP_FRONT, // list.pop_front()
    LIST_POP_BACK,  // list.pop_back()
    LIST_GET,       // list[index]
    LIST_SIZE,      // list.size()

    // ── I/O ──────────────────────────────────────────────────────────
    PRINT,          // print()
    
    // ── Error Handling ──────────────────────────────────────────────
    THROW,          // throw error
    TRY_START,      // Start try block
    TRY_END,        // End try block
    CATCH_START,    // Start catch block
    
    // ── Termination ──────────────────────────────────────────────────
    HALT            // Stop execution
};