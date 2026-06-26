#include "NovaVM.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include "OpCodes.h" 
#include <sstream>
#include "../Environment.h" 
std::string getNovaType(const NovaValue& val) {
    return std::visit([](auto&& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return "null";
        } else if constexpr (std::is_same_v<T, bool>) {
            return "bool";
        } else if constexpr (std::is_same_v<T, double>) {
            return "number";
        } else if constexpr (std::is_same_v<T, std::string>) {
            return "string";
        } else if constexpr (std::is_same_v<T, std::shared_ptr<NovaBunch>>) {
            return "bunch";
        } else if constexpr (std::is_same_v<T, std::shared_ptr<NovaMap>>) {
            return "map";
        } else if constexpr (std::is_same_v<T, std::shared_ptr<NovaStack>>) {
            return "stack";
        } else if constexpr (std::is_same_v<T, std::shared_ptr<NovaQueue>>) {
            return "queue";
        } else if constexpr (std::is_same_v<T, std::shared_ptr<NovaHeap>>) {
            return "heap";
        } else if constexpr (std::is_same_v<T, std::shared_ptr<NovaLinkedList>>) {
            return "linkedlist";
        } else if constexpr (std::is_same_v<T, std::shared_ptr<NovaFunction>>) {
            return "function";
        }
        return "unknown";
    }, val);
}

NovaVM::NovaVM() : pc(0), running(false), debug_mode(false) {
    current_env = std::make_shared<Environment>();
}
void NovaVM::run(const BytecodeProgram& program) {
   // std::cout << "[VM] run() called\n";
    code = program.code;
    constants = program.constants;
    string_constants = program.string_constants;
    function_addresses = program.function_addresses;  
    function_params = program.function_params;       
    //std::cout << "[VM] run: function_params size = " << function_params.size() << "\n";
    // for (const auto& [name, params] : function_params) {
    //     std::cout << "[VM] run: " << name << " has " << params.size() << " params\n";
    // }
    pc = 0;
    running = true;
    Frame frame;
    frame.pc = 0;
    frame.env = current_env;
    frames.push_back(frame);
    
    if (debug_mode) {
        //std::cout << "=== VM Debug: Starting ===\n";
    }
    
    //std::cout << "[VM] About to call executeLoop()\n";  
    executeLoop();
    //std::cout << "[VM] executeLoop() returned\n";  
 // ── Clear stack after execution ──────
    while (!stack.empty()) {
        pop();
    }
}

void NovaVM::executeLoop() {
    // << "[VM] PC=" << pc << " opcode=" << (int)code[pc] << " stack_size=" << stack.size() << "\n";
    int instruction_count = 0;
    const int MAX_INSTRUCTIONS = 100000;  
    while (running && pc < code.size()) {
        instruction_count++;
        if (instruction_count > MAX_INSTRUCTIONS) {
            std::cout << "[VM] ERROR: Infinite loop detected! Stopping.\n";
            running = false;
            break;
        }
        if (debug_mode) printDebugInfo();
        
        OpCode op = static_cast<OpCode>(code[pc++]);
        
        switch (op) {
            case OpCode::PUSH: {
                // Read the constant index
                if (pc >= code.size()) {
                    std::cout << "[VM] ERROR: PC out of bounds\n";
                    running = false;
                    break;
                }
                
                size_t idx = code[pc++];
                
                // ── Bounds check ──────────────────────────────────────────────────────────
                if (idx >= constants.size()) {
                    std::cout << "[VM] ERROR: Constant index " << idx << " out of bounds (size=" << constants.size() << ")\n";
                    running = false;
                    break;
                }
                
                BytecodeValue val = constants[idx];
                
                if (val.type == BytecodeValue::NUMBER) {
                    push(val.numVal);
                    //std::cout << "[VM] PUSH: number " << val.numVal << "\n";
                } else if (val.type == BytecodeValue::STRING) {
                    if (val.string_idx >= string_constants.size()) {
                        std::cout << "[VM] ERROR: String index " << val.string_idx << " out of bounds (size=" << string_constants.size() << ")\n";
                        running = false;
                        break;
                    }
                    const std::string& str = string_constants[val.string_idx];
                    push(str);
                   // std::cout << "[VM] PUSH: string '" << str << "'\n";
                } else if (val.type == BytecodeValue::BOOL) {
                    push(val.boolVal);
                    //std::cout << "[VM] PUSH: bool " << (val.boolVal ? "true" : "false") << "\n";
                } else if (val.type == BytecodeValue::LABEL) {
                    push((double)val.labelPos);
                } else {
                    std::cout << "[VM] ERROR: Unknown constant type: " << val.type << "\n";
                    running = false;
                    break;
                }
                break;
            }
            
            case OpCode::POP: {
                pop();
                break;
            }
            
            case OpCode::ADD: {
                auto right = pop();
                auto left = pop();
                
                //std::cout << "[VM] ADD: left type=" << left.index() << ", right type=" << right.index() << "\n";
                //std::cout << "[VM] ADD: left=" << novaStr(left) << ", right=" << novaStr(right) << "\n";
                
                try {
                    if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                        double result = std::get<double>(left) + std::get<double>(right);
                        //std::cout << "[VM] ADD: double result=" << result << "\n";
                        push(result);
                    } else if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right)) {
                        std::string result = novaStr(left) + novaStr(right);
                        //std::cout << "[VM] ADD: string result=" << result << "\n";
                        push(result);
                    } else {
                        //std::cout << "[VM] ADD: fallback result=0\n";
                        push(0.0);
                    }
                } catch (const std::exception& e) {
                    std::cout << "[VM] ADD: ERROR - " << e.what() << "\n";
                    push(0.0);
                }
                break;
            }
            
            case OpCode::SUB: {
                auto right = pop();
                auto left = pop();
               //std::cout << "[VM] SUB: " << novaStr(left) << " - " << novaStr(right) << "\n";
                if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                    double result = std::get<double>(left) - std::get<double>(right);
                   // std::cout << "[VM] SUB: result=" << result << "\n";
                    push(result);
                } else {
                    push(0.0);
                }
                break;
}
            
            case OpCode::MUL: {
                auto right = pop();
                auto left = pop();
                push(std::get<double>(left) * std::get<double>(right));
                break;
            }
            
            case OpCode::DIV: {
                auto right = pop();
                auto left = pop();
                double divisor = std::get<double>(right);
                if (divisor == 0) throw std::runtime_error("Division by zero");
                push(std::get<double>(left) / divisor);
                break;
            }
            
            case OpCode::MOD: {
                auto right = pop();
                auto left = pop();
                push(std::fmod(std::get<double>(left), std::get<double>(right)));
                break;
            }
            
            case OpCode::POW: {
                auto right = pop();
                auto left = pop();
                push(std::pow(std::get<double>(left), std::get<double>(right)));
                break;
            }
            
            case OpCode::NEGATE: {
                auto val = pop();
                push(-std::get<double>(val));
                break;
            }
            
            case OpCode::EQ: {
                auto right = pop();
                auto left = pop();
                bool result = (left == right);
                //std::cout << "[VM] EQ: " << novaStr(left) << " == " << novaStr(right) << " = " << result << "\n";
                push(result);
                break;
            }
            
            case OpCode::NEQ: {
                auto right = pop();
                auto left = pop();
                push(left != right);
                break;
            }
            
            case OpCode::LT: {
                auto right = pop();
                auto left = pop();
                
                double leftVal = std::get<double>(left);
                double rightVal = std::get<double>(right);
                bool result = leftVal < rightVal;
                
               // std::cout << "[VM] LT: left=" << leftVal << " (popped second), right=" << rightVal << " (popped first)\n";
                //std::cout << "[VM] LT: " << leftVal << " < " << rightVal << " = " << result << "\n";
                push(result);
                break;
            }
            
            case OpCode::GT: {
                auto right = pop();
                auto left = pop();
                push(std::get<double>(left) > std::get<double>(right));
                break;
            }
            
            case OpCode::LTE: {
                auto right = pop();
                auto left = pop();
                double leftVal = std::get<double>(left);
                double rightVal = std::get<double>(right);
                bool result = leftVal <= rightVal;
                            //std::cout << "[VM] LTE: " << leftVal << " <= " << rightVal << " = " << result << "\n";
                            push(result);
                            break;
                        }
            
            case OpCode::GTE: {
                auto right = pop();
                auto left = pop();
                push(std::get<double>(left) >= std::get<double>(right));
                break;
            }
            
            case OpCode::AND: {
                auto right = pop();
                auto left = pop();
                push(isTruthy(left) && isTruthy(right));
                break;
            }
            
            case OpCode::OR: {
                auto right = pop();
                auto left = pop();
                push(isTruthy(left) || isTruthy(right));
                break;
            }
            
            case OpCode::NOT: {
                auto val = pop();
                push(!isTruthy(val));
                break;
            }
            
            case OpCode::LOAD: {
                size_t idx = code[pc++];
                
                if (idx >= constants.size()) {
                    std::cout << "[VM] ERROR: LOAD constant index " << idx << " out of bounds\n";
                    running = false;
                    break;
                }
                
                BytecodeValue constVal = constants[idx];
                if (constVal.type != BytecodeValue::STRING) {
                    std::cout << "[VM] ERROR: LOAD requires string constant, got type " << constVal.type << "\n";
                    running = false;
                    break;
                }
                
                if (constVal.string_idx >= string_constants.size()) {
                    std::cout << "[VM] ERROR: String index " << constVal.string_idx << " out of bounds\n";
                    running = false;
                    break;
                }
                
                std::string name = string_constants[constVal.string_idx];
                //std::cout << "[VM] LOAD: trying to load '" << name << "'\n";
                //std::cout << "[VM] LOAD: " << name << " (env=" << current_env.get() << ")\n";               
                try {
                    NovaValue val = current_env->get(name);
                    push(val);
                    //std::cout << "[VM] LOAD: loaded '" << name << "' = " << novaStr(val) << "\n";
                } catch (const std::runtime_error& e) {
                    std::cout << "[VM] LOAD: ERROR - " << e.what() << "\n";
                    running = false;
                    break;
                }
                break;
            }
                        
            case OpCode::STORE: {
                size_t idx = code[pc++];
                
                // ── Get the constant ──────────────────────────────────────────────────────
                if (idx >= constants.size()) {
                    std::cout << "[VM] ERROR: STORE constant index " << idx << " out of bounds\n";
                    running = false;
                    break;
                }
                
                BytecodeValue constVal = constants[idx];
                if (constVal.type != BytecodeValue::STRING) {
                    std::cout << "[VM] ERROR: STORE requires string constant, got type " << constVal.type << "\n";
                    running = false;
                    break;
                }
                
                // ── Get the string from the string pool ──────────────────────────────────
                if (constVal.string_idx >= string_constants.size()) {
                    std::cout << "[VM] ERROR: String index " << constVal.string_idx << " out of bounds\n";
                    running = false;
                    break;
                }
                
                std::string name = string_constants[constVal.string_idx];
                auto val = pop();
                current_env->set(name, val);
                break;
            }
            
            case OpCode::DEFINE: {
                size_t idx = code[pc++];
                
                // ── Get the constant ──────────────────────────────────────────────────────
                if (idx >= constants.size()) {
                    std::cout << "[VM] ERROR: DEFINE constant index " << idx << " out of bounds\n";
                    running = false;
                    break;
                }
                
                BytecodeValue constVal = constants[idx];
                if (constVal.type != BytecodeValue::STRING) {
                    std::cout << "[VM] ERROR: DEFINE requires string constant, got type " << constVal.type << "\n";
                    running = false;
                    break;
                }
                
                // ── Get the string from the string pool ──────────────────────────────────
                if (constVal.string_idx >= string_constants.size()) {
                    std::cout << "[VM] ERROR: String index " << constVal.string_idx << " out of bounds\n";
                    running = false;
                    break;
                }
                std::string name = string_constants[constVal.string_idx];
                auto val = pop();
                //std::cout << "[VM] DEFINE: " << name << " = " << novaStr(val)  << " (env=" << current_env.get() << ")\n";
                current_env->define(name, val);
                current_env->varTypes[name] = ::getNovaType(val);
                break;
            }
                        
            case OpCode::JUMP: {
                size_t target = code[pc] | (code[pc+1] << 8) | (code[pc+2] << 16);
                pc += 3;
                pc = target;
                break;
        }
            
            case OpCode::JUMP_IF: {
                size_t target = code[pc] | (code[pc+1] << 8) | (code[pc+2] << 16);
                pc += 3;
                auto val = pop();
                if (isTruthy(val)) pc = target;
                break;
            }
            
           case OpCode::JUMP_IF_NOT: {
                size_t target = code[pc] | (code[pc+1] << 8) | (code[pc+2] << 16);
                pc += 3;
                auto val = pop();
                
                bool truthy = isTruthy(val);
                //std::cout << "[VM] JUMP_IF_NOT: val=" << novaStr(val) << ", truthy=" << truthy << ", target=" << target << "\n";
                
                if (!truthy) {
                    //std::cout << "[VM] JUMP_IF_NOT: JUMPING to " << target << "\n";
                    pc = target;
                } else {
                    //std::cout << "[VM] JUMP_IF_NOT: NOT jumping, continuing at " << pc << "\n";
                }
                break;
            }
            
            case OpCode::CALL: {
                //std::cout << "[VM] CALL: ENTERING CALL\n";
                size_t idx = code[pc++];
                BytecodeValue constVal = constants[idx];
                if (constVal.type != BytecodeValue::STRING) {
                    //std::cout << "[VM] ERROR: CALL requires string constant\n";
                    running = false;
                    break;
                }
                std::string name = string_constants[constVal.string_idx];
                
                //std::cout << "[VM] CALL: function name = '" << name << "'\n";
                //std::cout << "[VM] CALL: function_params size = " << function_params.size() << "\n";
                
                // ── Get argument count ──────────────────────────────────────────────────
                auto argc_val = pop();
                size_t argc = static_cast<size_t>(std::get<double>(argc_val));
                //std::cout << "[VM] CALL: argc = " << argc << "\n";
                
                // ── Collect arguments ──────────────────────────────────────────────────
                std::vector<NovaValue> args;
                for (size_t i = 0; i < argc; ++i) {
                    args.push_back(pop());
                }
                std::reverse(args.begin(), args.end());
                
                // for (size_t i = 0; i < args.size(); ++i) {
                //     //std::cout << "[VM] CALL: arg" << i << " = " << novaStr(args[i]) << "\n";
                // }
                
                // ── Look up function address ──────────────────────────────────────────
                auto it = function_addresses.find(name);
                if (it == function_addresses.end()) {
                    std::cout << "[VM] ERROR: Function not found: " << name << "\n";
                    running = false;
                    break;
                }
                size_t func_addr = it->second;
                //std::cout << "[VM] CALL: func_addr = " << func_addr << "\n";
                
                // ── Get parameter names ──────────────────────────────────────────────
                auto params_it = function_params.find(name);
                if (params_it == function_params.end()) {
                    std::cout << "[VM] ERROR: Function params not found: " << name << "\n";
                    running = false;
                    break;
                }
                const std::vector<std::string>& params = params_it->second;
                
                //std::cout << "[VM] CALL: params = ";
                // for (const auto& p : params) std::cout << p << " ";
                // std::cout << "\n";
                
                // ── Push frame ──────────────────────────────────────────────────────────
                //std::cout << "[VM] CALL: Pushing frame, current pc = " << pc << "\n";
              //  std::cout << "[VM] CALL: " << name << " | old_env=" << current_env.get() << ", pc=" << pc << "\n";

                Frame frame;
                frame.pc = pc;
                frame.env = std::make_shared<Environment>(current_env);
                frames.push_back(frame);
                current_env = frame.env;
                //std::cout << "[VM] CALL: Frame pushed\n";
               // std::cout << "[VM] CALL: " << name << " | new_env=" << current_env.get() << ", func_addr=" << func_addr << "\n";
                // ── Bind arguments to parameters ──────────────────────────────────────
                //std::cout << "[VM] CALL: Binding " << params.size() << " params\n";
                for (size_t i = 0; i < params.size(); ++i) {
                    if (i < args.size()) {
                        //std::cout << "[VM] CALL: binding " << params[i] << " = " << novaStr(args[i]) << "\n";
                        current_env->define(params[i], args[i]);
                    } else {
                        //std::cout << "[VM] CALL: binding " << params[i] << " = 0 (default)\n";
                        current_env->define(params[i], 0.0);
                    }
                }
                
                // ── Debug: print all variables in the new environment ──────────────────
               // std::cout << "[VM] CALL: Environment variables after binding:\n";
                // for (const auto& [var_name, var_value] : current_env->vars) {
                //     std::cout << "  " << var_name << " = " << novaStr(var_value) << "\n";
                // }
                
                //std::cout << "[VM] CALL: Jumping to " << func_addr << "\n";
                pc = func_addr;
                break;
            }
            
            case OpCode::RETURN: {
                //std::cout << "[VM] RETURN\n";
                
                if (stack.empty()) {
                    std::cout << "[VM] ERROR: RETURN: stack empty\n";
                    running = false;
                    break;
                }
                
                auto val = pop();
                //std::cout << "[VM] RETURN: returning " << novaStr(val) << "\n";
                
                if (frames.empty()) {
                    std::cout << "[VM] ERROR: Return with no frame\n";
                    running = false;
                    break;
                }
                
                // ── Restore frame ──────────────────────────────────────────────────────
                Frame frame = frames.back();
                frames.pop_back();
                current_env = frame.env;  // ← This MUST restore the environment!
                pc = frame.pc;            // ← This MUST restore the program counter!
                
                //std::cout << "[VM] RETURN: restored to env=" << current_env.get() << ", pc=" << pc << "\n";
                
                // ── Push return value ──────────────────────────────────────────────────
                push(val);
                break;
            }
            
            case OpCode::SLICE: {
                auto step_val = pop();
                auto end_val = pop();
                auto start_val = pop();
                auto obj = pop();
                
                size_t start = 0, end = 0, step = 1;
                
                // ── Get start ──────────────────────────────────────────────────────
                if (std::holds_alternative<double>(start_val)) {
                    start = static_cast<size_t>(std::get<double>(start_val));
                } else {
                    //std::cout << "[VM] SLICE: start type=" << start_val.index() << "\n";
                    start = 0;
                }
                
                // ── Get end (if -1, use size) ──────────────────────────────────────
                if (std::holds_alternative<double>(end_val)) {
                    end = static_cast<size_t>(std::get<double>(end_val));
                } else {
                    //std::cout << "[VM] SLICE: end type=" << end_val.index() << "\n";
                    end = 0;
                }
                
                // ── Get step ──────────────────────────────────────────────────────
                if (std::holds_alternative<double>(step_val)) {
                    step = static_cast<size_t>(std::get<double>(step_val));
                } else {
                   // std::cout << "[VM] SLICE: step type=" << step_val.index() << "\n";
                    step = 1;
                }
                if (step == 0) step = 1;
                
                // ── Handle Bunch ──────────────────────────────────────────────────
                if (std::holds_alternative<std::shared_ptr<NovaBunch>>(obj)) {
                    auto bunch = std::get<std::shared_ptr<NovaBunch>>(obj);
                    size_t size = bunch->size();
                    // If end == -1 sentinel, use size
                    if (end == static_cast<size_t>(-1)) end = size;
                    if (start > size) start = size;
                    if (end > size) end = size;
                    auto result = std::make_shared<NovaBunch>();
                    for (size_t i = start; i < end; i += step) {
                        result->push((*bunch)[i]);
                    }
                    push(result);
                    break;
                }
                
                // ── Handle String (shared_ptr<NovaString>) ──────────────────────
                if (std::holds_alternative<std::shared_ptr<NovaString>>(obj)) {
                    auto str = std::get<std::shared_ptr<NovaString>>(obj);
                    size_t size = str->data.size();
                    if (end == static_cast<size_t>(-1)) end = size;
                    if (start > size) start = size;
                    if (end > size) end = size;
                    std::string result;
                    for (size_t i = start; i < end; i += step) {
                        result += str->data[i];
                    }
                    push(result);
                    break;
                }
                
                // ── Handle String (std::string) ──────────────────────────────────
                if (std::holds_alternative<std::string>(obj)) {
                    const std::string& str = std::get<std::string>(obj);
                    size_t size = str.size();
                    if (end == static_cast<size_t>(-1)) end = size;
                    if (start > size) start = size;
                    if (end > size) end = size;
                    std::string result;
                    for (size_t i = start; i < end; i += step) {
                        result += str[i];
                    }
                    push(result);
                    break;
                }
                
                //std::cout << "[VM] SLICE: object type=" << obj.index() << "\n";
                throw std::runtime_error("SLICE: Object is not a bunch or string");
                break;
            }

            case OpCode::BUNCH_INIT: {
                push(std::make_shared<NovaBunch>());
                break;
            }
            
            case OpCode::BUNCH_PUSH: {
                auto val = pop();        // Value to push
                auto obj = pop();        // Bunch object
                if (!std::holds_alternative<std::shared_ptr<NovaBunch>>(obj)) {
                    throw std::runtime_error("BUNCH_PUSH: Object is not a bunch");
                }
                auto bunch = std::get<std::shared_ptr<NovaBunch>>(obj);
                bunch->push(val);
                push(obj); 
                break;
            }
            
            case OpCode::BUNCH_POP: {
                auto obj = pop();
                auto bunch = std::get<std::shared_ptr<NovaBunch>>(obj);
                push(bunch->pop());
                break;
            }
            
           case OpCode::BUNCH_GET: {
                auto idx_val = pop();
                auto obj = pop();
                
                // ── Handle Bunch ──────────────────────────────────────────────────────────
                if (std::holds_alternative<std::shared_ptr<NovaBunch>>(obj)) {
                    auto bunch = std::get<std::shared_ptr<NovaBunch>>(obj);
                    size_t idx = static_cast<size_t>(std::get<double>(idx_val));
                    if (idx >= bunch->size()) {
                        throw std::runtime_error("Bunch index out of bounds");
                    }
                    push(bunch->at(idx));
                    break;
                }
                
                // ── Handle String ──────────────────────────────────────────────────────────
                if (std::holds_alternative<std::shared_ptr<NovaString>>(obj)) {
                    auto str = std::get<std::shared_ptr<NovaString>>(obj);
                    size_t idx = static_cast<size_t>(std::get<double>(idx_val));
                    if (idx >= str->data.size()) {
                        throw std::runtime_error("String index out of bounds");
                    }
                    push(std::string(1, str->data[idx]));
                    break;
                }
                
                throw std::runtime_error("BUNCH_GET: Object is not a bunch or string");
            }
            
            case OpCode::BUNCH_SIZE: {
                auto obj = pop();
                auto bunch = std::get<std::shared_ptr<NovaBunch>>(obj);
                push((double)bunch->size());
                break;
            }
            
            case OpCode::BUNCH_EMPTY: {
                auto obj = pop();
                auto bunch = std::get<std::shared_ptr<NovaBunch>>(obj);
                push(bunch->empty());
                break;
            }
            case OpCode::STRING_INIT: {
                push(std::make_shared<NovaString>());
                break;
            }

            case OpCode::STRING_SET: {
                auto char_val = pop();
                auto idx_val = pop();
                auto obj = pop();
                
                if (!std::holds_alternative<double>(idx_val)) {
                    throw std::runtime_error("STRING_SET: Index must be a number");
                }
                if (!std::holds_alternative<std::string>(char_val)) {
                    throw std::runtime_error("STRING_SET: Value must be a string (single character)");
                }
                
                size_t idx = static_cast<size_t>(std::get<double>(idx_val));
                std::string charStr = std::get<std::string>(char_val);
                
                // ── Handle NovaString (shared_ptr) ──────────────────────────────────────
                if (std::holds_alternative<std::shared_ptr<NovaString>>(obj)) {
                    auto str = std::get<std::shared_ptr<NovaString>>(obj);
                    if (idx >= str->data.size()) {
                        throw std::runtime_error("String index out of bounds");
                    }
                    if (charStr.size() != 1) {
                        throw std::runtime_error("STRING_SET: Must set a single character");
                    }
                    str->data[idx] = charStr[0];
                    break;
                }
                
                // ── Handle std::string ──────────────────────────────────────────────────
                if (std::holds_alternative<std::string>(obj)) {
                    // std::string is immutable in our VM (we can modify it)
                    // But we need to convert to NovaString first
                    std::string& str = std::get<std::string>(obj);
                    if (idx >= str.size()) {
                        throw std::runtime_error("String index out of bounds");
                    }
                    if (charStr.size() != 1) {
                        throw std::runtime_error("STRING_SET: Must set a single character");
                    }
                    str[idx] = charStr[0];
                    break;
                }
                
                throw std::runtime_error("STRING_SET: Object is not a string");
                break;
            }
            case OpCode::STRING_GET: {
                auto idx_val = pop();
                auto obj = pop();
                
                if (!std::holds_alternative<double>(idx_val)) {
                    throw std::runtime_error("STRING_GET: Index must be a number");
                }
                size_t idx = static_cast<size_t>(std::get<double>(idx_val));
                
                // ── Handle NovaString (shared_ptr) ──────────────────────────────────────
                if (std::holds_alternative<std::shared_ptr<NovaString>>(obj)) {
                    auto str = std::get<std::shared_ptr<NovaString>>(obj);
                    if (idx >= str->data.size()) {
                        throw std::runtime_error("String index out of bounds");
                    }
                    push(std::string(1, str->data[idx]));
                    break;
                }
                
                // ── Handle std::string ──────────────────────────────────────────────────
                if (std::holds_alternative<std::string>(obj)) {
                    const std::string& str = std::get<std::string>(obj);
                    if (idx >= str.size()) {
                        throw std::runtime_error("String index out of bounds");
                    }
                    push(std::string(1, str[idx]));
                    break;
                }
                
                throw std::runtime_error("STRING_GET: Object is not a string");
                break;
            }

            case OpCode::STRING_SIZE: {
                auto obj = pop();
                
                // ── Handle NovaString (shared_ptr) ──────────────────────────────────────
                if (std::holds_alternative<std::shared_ptr<NovaString>>(obj)) {
                    auto str = std::get<std::shared_ptr<NovaString>>(obj);
                    push((double)str->data.size());
                    break;
                }
                
                // ── Handle std::string ──────────────────────────────────────────────────
                if (std::holds_alternative<std::string>(obj)) {
                    const std::string& str = std::get<std::string>(obj);
                    push((double)str.size());
                    break;
                }
                
                throw std::runtime_error("STRING_SIZE: Object is not a string");
                break;
            }
            case OpCode::MAP_INIT: {
                push(std::make_shared<NovaMap>());
                break;
            }
            
            case OpCode::MAP_SET: {
                auto val = pop();        // value
                auto key_val = pop();    // key
                auto obj = pop();        // map object
                
                // ── TYPE CHECKING ──────────────────────────────────────────────────────
                if (!std::holds_alternative<std::shared_ptr<NovaMap>>(obj)) {
                    throw std::runtime_error("MAP_SET: First argument must be a map");
                }
                if (!std::holds_alternative<std::string>(key_val)) {
                    throw std::runtime_error("MAP_SET: Key must be a string");
                }
                
                auto map = std::get<std::shared_ptr<NovaMap>>(obj);
                map->set(std::get<std::string>(key_val), val);
                push(obj);  // Push map back for chaining
                break;
            }
            
            case OpCode::MAP_GET: {
                auto key_val = pop();
                auto obj = pop();
                auto map = std::get<std::shared_ptr<NovaMap>>(obj);
                push(map->get(std::get<std::string>(key_val)));
                break;
            }
            
            case OpCode::MAP_HAS: {
                auto key_val = pop();
                auto obj = pop();
                auto map = std::get<std::shared_ptr<NovaMap>>(obj);
                push(map->has(std::get<std::string>(key_val)));
                break;
            }
            
            case OpCode::MAP_SIZE: {
                auto obj = pop();
                auto map = std::get<std::shared_ptr<NovaMap>>(obj);
                push((double)map->size());
                break;
            }
            case OpCode::MAP_KEYS: {
                auto obj = pop();
                if (!std::holds_alternative<std::shared_ptr<NovaMap>>(obj)) {
                    throw std::runtime_error("MAP_KEYS: Object is not a map");
                }
                auto map = std::get<std::shared_ptr<NovaMap>>(obj);
                auto keys = map->keys();
                auto result = std::make_shared<NovaBunch>();
                for (const auto& key : keys) {
                    result->push(key);
                }
                push(result);
                break;
            }
            case OpCode::STACK_INIT: {
                push(std::make_shared<NovaStack>());
                break;
            }
            
            case OpCode::STACK_PUSH: {
                auto val = pop();
                auto obj = pop();
                auto stack = std::get<std::shared_ptr<NovaStack>>(obj);
                stack->push(val);
                push(obj);
                break;
            }
            
            case OpCode::STACK_POP: {
                auto obj = pop();
                auto stack = std::get<std::shared_ptr<NovaStack>>(obj);
                push(stack->pop());
                break;
            }
            
            case OpCode::STACK_TOP: {
                auto obj = pop();
                auto stack = std::get<std::shared_ptr<NovaStack>>(obj);
                push(stack->top());
                break;
            }
            
            case OpCode::STACK_SIZE: {
                auto obj = pop();
                auto stack = std::get<std::shared_ptr<NovaStack>>(obj);
                push((double)stack->size());
                break;
            }
            
            case OpCode::QUEUE_INIT: {
                push(std::make_shared<NovaQueue>());
                break;
            }
            
            case OpCode::QUEUE_ENQUEUE: {
                auto val = pop();
                auto obj = pop();
                auto queue = std::get<std::shared_ptr<NovaQueue>>(obj);
                queue->enqueue(val);
                push(obj);
                break;
            }
            
            case OpCode::QUEUE_DEQUEUE: {
                auto obj = pop();
                auto queue = std::get<std::shared_ptr<NovaQueue>>(obj);
                push(queue->dequeue());
                break;
            }
            
            case OpCode::QUEUE_FRONT: {
                auto obj = pop();
                auto queue = std::get<std::shared_ptr<NovaQueue>>(obj);
                push(queue->front());
                break;
            }
            
            case OpCode::QUEUE_SIZE: {
                auto obj = pop();
                auto queue = std::get<std::shared_ptr<NovaQueue>>(obj);
                push((double)queue->size());
                break;
            }
            
            case OpCode::HEAP_INIT: {
                push(std::make_shared<NovaHeap>());
                break;
            }
            
            case OpCode::HEAP_PUSH: {
                auto val = pop();
                auto obj = pop();
                auto heap = std::get<std::shared_ptr<NovaHeap>>(obj);
                heap->push(std::get<double>(val));
                push(obj);
                break;
            }
            
            case OpCode::HEAP_POP: {
                auto obj = pop();
                auto heap = std::get<std::shared_ptr<NovaHeap>>(obj);
                push(heap->pop());
                break;
            }
            
            case OpCode::HEAP_TOP: {
                auto obj = pop();
                auto heap = std::get<std::shared_ptr<NovaHeap>>(obj);
                push(heap->top());
                break;
            }
            
            case OpCode::HEAP_SIZE: {
                auto obj = pop();
                auto heap = std::get<std::shared_ptr<NovaHeap>>(obj);
                push((double)heap->size());
                break;
            }
            
            case OpCode::LIST_INIT: {
                push(std::make_shared<NovaLinkedList>());
                break;
            }
            
            case OpCode::LIST_PUSH_FRONT: {
                auto val = pop();
                auto obj = pop();
                auto list = std::get<std::shared_ptr<NovaLinkedList>>(obj);
                list->push_front(val);
                push(obj);
                break;
            }
            
            case OpCode::LIST_PUSH_BACK: {
                auto val = pop();
                auto obj = pop();
                auto list = std::get<std::shared_ptr<NovaLinkedList>>(obj);
                list->push_back(val);
                push(obj);
                break;
            }
            
            case OpCode::LIST_POP_FRONT: {
                auto obj = pop();
                auto list = std::get<std::shared_ptr<NovaLinkedList>>(obj);
                push(list->pop_front());
                break;
            }
            
            case OpCode::LIST_POP_BACK: {
                auto obj = pop();
                auto list = std::get<std::shared_ptr<NovaLinkedList>>(obj);
                push(list->pop_back());
                break;
            }
            
            case OpCode::LIST_GET: {
                auto idx_val = pop();
                auto obj = pop();
                auto list = std::get<std::shared_ptr<NovaLinkedList>>(obj);
                size_t idx = static_cast<size_t>(std::get<double>(idx_val));
                push(list->at(idx));
                break;
            }
            
            case OpCode::LIST_SIZE: {
                auto obj = pop();
                auto list = std::get<std::shared_ptr<NovaLinkedList>>(obj);
                push((double)list->size());
                break;
            }
            
            case OpCode::PRINT: {
                //std::cout << "[VM] PRINT: stack_size=" << stack.size() << "\n";
                
                // ── Get argument count ──────────────────────────────────────────────────
                auto count_val = pop();
                size_t count = static_cast<size_t>(std::get<double>(count_val));
                //std::cout << "[VM] PRINT: " << count << " arguments\n";
                
                // ── Get end ──────────────────────────────────────────────────────────────
                auto end = pop();
                std::string endStr = novaStr(end);  // ← Convert to string!
                //std::cout << "[VM] PRINT: end='" << endStr << "'\n";
                
                // ── Get sep ──────────────────────────────────────────────────────────────
                auto sep = pop();
                std::string sepStr = novaStr(sep);  // ← Convert to string!
                //std::cout << "[VM] PRINT: sep='" << sepStr << "'\n";
                
                // ── Collect arguments (they're on the stack in reverse order) ──────────
                std::vector<NovaValue> args;
                for (size_t i = 0; i < count; ++i) {
                    args.push_back(pop());
                }
                //std::cout << "[VM] PRINT: collected " << args.size() << " args\n";
                
                // ── Print with sep and end ──────────────────────────────────────────────
                // Args are in reverse order (last argument first)
                for (int i = static_cast<int>(args.size()) - 1; i >= 0; --i) {
                    if (i < static_cast<int>(args.size()) - 1) {
                        std::cout << sepStr;
                    }
                    std::cout << novaStr(args[i]);
                }
                std::cout << endStr << std::flush;
                
                //std::cout << "[VM] PRINT: done\n";
                break;
            }
            
            case OpCode::TRY_START: {
                size_t catch_addr = code[pc] | (code[pc+1] << 8) | (code[pc+2] << 16);
                pc += 3;
                try_stack.push(catch_addr);
                //std::cout << "[VM] TRY_START: catch at " << catch_addr << "\n";
                break;
            }

            case OpCode::TRY_END: {
                if (!try_stack.empty()) {
                    try_stack.pop();
                }
                //std::cout << "[VM] TRY_END\n";
                break;
            }

            case OpCode::CATCH_START: {
                // Just pop the try_stack, leave the error on the stack
                if (!try_stack.empty()) {
                    try_stack.pop();
                }
                //std::cout << "[VM] CATCH_START: stack_size=" << stack.size() << "\n";
                break;
            }

            case OpCode::THROW: {
                // The error value is already on the stack from throw()
                // Just check if there's a catch block
                //std::cout << "[VM] THROW: stack_size=" << stack.size() << "\n";
                
                if (stack.empty()) {
                    std::cout << "[VM] THROW: ERROR - Stack empty!\n";
                    running = false;
                    break;
                }
                
                if (!try_stack.empty()) {
                    size_t catch_addr = try_stack.top();
                    try_stack.pop();
                    //std::cout << "[VM] THROW: Jumping to catch at " << catch_addr << "\n";
                    // The error stays on the stack for CATCH_START to pop
                    pc = catch_addr;
                    break;
                }
                
                // No catch handler - pop and print error
                auto val = pop();
                std::cout << "Uncaught error: " << novaStr(val) << "\n";
                running = false;
                break;
}
            case OpCode::HALT:
                running = false;
                break;
            
            default:
                throw std::runtime_error("Unknown opcode: " + std::to_string(static_cast<int>(op)));
        }
    }
   // std::cout<<"VM ENDED"<<std::endl;
}

void NovaVM::push(const NovaValue& val) {
    stack.push_back(val);
}

NovaValue NovaVM::pop() {
    if (stack.empty()) throw std::runtime_error("Stack underflow");
    NovaValue val = stack.back();
    stack.pop_back();
    return val;
}

NovaValue NovaVM::peek(size_t offset) {
    if (offset >= stack.size()) throw std::runtime_error("Stack underflow");
    return stack[stack.size() - 1 - offset];
}

void NovaVM::pushFrame() {
    Frame frame;
    frame.pc = pc;
    frame.env = std::make_shared<Environment>(current_env);
    frames.push_back(frame);
    current_env = frame.env;
}

void NovaVM::popFrame() {
    if (frames.empty()) throw std::runtime_error("No frame to pop");
    frames.pop_back();
    if (frames.empty()) {
        current_env = std::make_shared<Environment>();
    } else {
        current_env = frames.back().env;
        pc = frames.back().pc;
    }
}

void NovaVM::callFunction(const std::string& name, size_t argc) {
    // For now, just push a frame
    // Later add proper function lookup
    pushFrame();
}

void NovaVM::returnFunction() {
    popFrame();
}

bool NovaVM::isTruthy(const NovaValue& val) {
    return std::visit([](auto&& v) -> bool {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) return false;
        else if constexpr (std::is_same_v<T, bool>) return v;
        else if constexpr (std::is_same_v<T, double>) return v != 0.0;
        else if constexpr (std::is_same_v<T, std::string>) return !v.empty();
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaBunch>>) return v && !v->empty();
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaMap>>) return v && !v->empty();
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaStack>>) return v && !v->empty();
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaQueue>>) return v && !v->empty();
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaHeap>>) return v && !v->empty();
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaLinkedList>>) return v && !v->empty();
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaFunction>>) return true;
        return false;
    }, val);
}

std::string NovaVM::novaStr(const NovaValue& val) {
        return std::visit([](auto&& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return "null";
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
        }
        else if constexpr (std::is_same_v<T, double>) {
            if (v == std::floor(v) && std::abs(v) < 1e15) {
                return std::to_string(static_cast<long long>(v));
            }
            std::ostringstream oss;
            oss << v;
            std::string result = oss.str();
            // Remove trailing zeros
            size_t dot = result.find('.');
            if (dot != std::string::npos) {
                while (result.back() == '0') {
                    result.pop_back();
                }
                if (result.back() == '.') {
                    result.pop_back();
                }
            }
            return result;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return v;
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaBunch>>) {
            std::string result = "[";
            for (size_t i = 0; i < v->data.size(); ++i) {
                if (i > 0) result += ", ";
                // ── Convert each element using novaStr ──
                // We need a helper function to convert NovaValue to string
                const NovaValue& elem = v->data[i];
                result += std::visit([](auto&& val) -> std::string {
                    using U = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<U, double>) {
                        if (val == std::floor(val) && std::abs(val) < 1e15)
                            return std::to_string(static_cast<long long>(val));
                        return std::to_string(val);
                    } else if constexpr (std::is_same_v<U, std::string>) {
                        return val;
                    } else if constexpr (std::is_same_v<U, bool>) {
                        return val ? "true" : "false";
                    } else if constexpr (std::is_same_v<U, std::nullptr_t>) {
                        return "null";
                    } else if constexpr (std::is_same_v<U, std::shared_ptr<NovaBunch>>) {
                        return "[bunch]";
                    } else {
                        return "<object>";
                    }
                }, elem);
            }
            result += "]";
            return result;
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaMap>>) {
            return "{map: " + std::to_string(v->size()) + "}";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaStack>>) {
            return "{stack: " + std::to_string(v->size()) + "}";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaQueue>>) {
            return "{queue: " + std::to_string(v->size()) + "}";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaHeap>>) {
            return "{heap: " + std::to_string(v->size()) + "}";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaLinkedList>>) {
            return "{list: " + std::to_string(v->size()) + "}";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<NovaFunction>>) {
            return "<function " + v->name + ">";
        }
        return "";
    }, val);
}

void NovaVM::printDebugInfo() {
    std::cout << "PC: " << pc << " | Stack: [";
    for (size_t i = 0; i < stack.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << novaStr(stack[i]);
    }
    std::cout << "]\n";
}