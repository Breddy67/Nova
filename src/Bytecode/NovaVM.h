#pragma once
#include "BytecodeProgram.h"
#include "../Environment.h" 

#include "../AST.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <memory>

class NovaVM {
public:
    NovaVM();  
    void run(const BytecodeProgram& program);
    void setDebug(bool enabled) { debug_mode = enabled; }
    std::string getNovaType(const NovaValue& val);
    
private:
    struct Frame {
        size_t pc;
        std::shared_ptr<Environment> env;
        size_t return_pc;
    };
    static constexpr int MAX_INSTRUCTIONS = 100000;
    std::vector<uint8_t> code;
    std::vector<BytecodeValue> constants;
    std::vector<NovaValue> stack;
    std::stack<size_t> try_stack; 
    std::unordered_map<std::string, size_t> function_addresses; 
    std::unordered_map<std::string, std::vector<std::string>> function_params; 
    std::vector<Frame> frames;
    std::shared_ptr<Environment> current_env;
    std::vector<std::string> string_constants;
    size_t pc;
    bool running;
    bool debug_mode;
    
    void push(const NovaValue& val);
    NovaValue pop();
    NovaValue peek(size_t offset = 0);
    void pushFrame();
    void popFrame();
    
    void executeLoop();
    void callFunction(const std::string& name, size_t argc);
    void returnFunction();
    
    bool isTruthy(const NovaValue& val);
    std::string novaStr(const NovaValue& val);
    void printDebugInfo();
    
    void handleBunchOp(OpCode op);
    void handleFuncOp(OpCode op);
    void handleStackOp(OpCode op);
    void handleQueueOp(OpCode op);
    void handleHeapOp(OpCode op);
    void handleListOp(OpCode op);
};