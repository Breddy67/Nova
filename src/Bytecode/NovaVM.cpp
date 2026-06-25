#include "NovaVM.h"
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
            return "func";
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
    pc = 0;
    running = true;
    //std::cout << "[VM] code size: " << code.size() << "\n";  
    //std::cout << "[VM] constants: " << constants.size() << "\n";  
    
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
                if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                    push(std::get<double>(left) + std::get<double>(right));
                } else {
                    push(novaStr(left) + novaStr(right));
                }
                break;
            }
            
            case OpCode::SUB: {
                auto right = pop();
                auto left = pop();
                push(std::get<double>(left) - std::get<double>(right));
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
                push(left == right);
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
                push(std::get<double>(left) < std::get<double>(right));
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
                push(std::get<double>(left) <= std::get<double>(right));
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
                
                // ── Get the constant ──────────────────────────────────────────────────────
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
                
                // ── Get the string from the string pool ──────────────────────────────────
                if (constVal.string_idx >= string_constants.size()) {
                    std::cout << "[VM] ERROR: String index " << constVal.string_idx << " out of bounds\n";
                    running = false;
                    break;
                }
                
                std::string name = string_constants[constVal.string_idx];
                push(current_env->get(name));
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
                if (!isTruthy(val)) pc = target;
                break;
            }
            
            case OpCode::CALL: {
                size_t argc = code[pc++];
                size_t idx = code[pc++];
                
                BytecodeValue constVal = constants[idx];
                if (constVal.type != BytecodeValue::STRING) {
                    std::cout << "[VM] ERROR: CALL requires string constant\n";
                    running = false;
                    break;
                }
                
                std::string name = string_constants[constVal.string_idx];
                callFunction(name, argc);
                break;
            }
            
            case OpCode::RETURN: {
                auto val = pop();
                popFrame();
                push(val);
                break;
            }
            
            case OpCode::SLICE: {
                auto step = pop();
                auto end = pop();
                auto start = pop();
                auto obj = pop();
                
                // ── Handle Bunch ──────────────────────────────────────────────────────
                if (std::holds_alternative<std::shared_ptr<NovaBunch>>(obj)) {
                    auto bunch = std::get<std::shared_ptr<NovaBunch>>(obj);
                    size_t s = static_cast<size_t>(std::get<double>(start));
                    size_t e = static_cast<size_t>(std::get<double>(end));
                    size_t st = static_cast<size_t>(std::get<double>(step));
                    if (s > bunch->size()) s = bunch->size();
                    if (e > bunch->size()) e = bunch->size();
                    if (st == 0) st = 1;
                    auto result = std::make_shared<NovaBunch>();
                    for (size_t i = s; i < e; i += st) {
                        result->push((*bunch)[i]);
                    }
                    push(result);
                    break;
                }
                
                // ── Handle String ──────────────────────────────────────────────────────
                if (std::holds_alternative<std::shared_ptr<NovaString>>(obj)) {
                    auto str = std::get<std::shared_ptr<NovaString>>(obj);
                    size_t s = static_cast<size_t>(std::get<double>(start));
                    size_t e = static_cast<size_t>(std::get<double>(end));
                    size_t st = static_cast<size_t>(std::get<double>(step));
                    if (s > str->data.size()) s = str->data.size();
                    if (e > str->data.size()) e = str->data.size();
                    if (st == 0) st = 1;
                    std::string result;
                    for (size_t i = s; i < e; i += st) {
                        result += str->data[i];
                    }
                    push(result);
                    break;
                }
                
                throw std::runtime_error("SLICE: Object is not a bunch or string");
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
                auto char_val = pop();   // The character to set
                auto idx_val = pop();    // The index
                auto obj = pop();        // The string object
                
                if (!std::holds_alternative<std::shared_ptr<NovaString>>(obj)) {
                    throw std::runtime_error("STRING_SET: Object is not a string");
                }
                if (!std::holds_alternative<double>(idx_val)) {
                    throw std::runtime_error("STRING_SET: Index must be a number");
                }
                if (!std::holds_alternative<std::string>(char_val)) {
                    throw std::runtime_error("STRING_SET: Value must be a string (single character)");
                }
                
                auto str = std::get<std::shared_ptr<NovaString>>(obj);
                size_t idx = static_cast<size_t>(std::get<double>(idx_val));
                std::string charStr = std::get<std::string>(char_val);
                
                // ── BOUNDS CHECK ──────────────────────────────────────────────────
                if (idx >= str->data.size()) {
                    throw std::runtime_error("String index out of bounds: " + std::to_string(idx) + 
                                            " (size: " + std::to_string(str->data.size()) + ")");
                }
                if (charStr.size() != 1) {
                    throw std::runtime_error("STRING_SET: Must set a single character");
                }
                
                str->data[idx] = charStr[0];
                break;
            }
            case OpCode::STRING_GET: {
                auto idx_val = pop();
                auto obj = pop();
                if (!std::holds_alternative<std::shared_ptr<NovaString>>(obj)) {
                    throw std::runtime_error("STRING_GET: Object is not a string");
                }
                if (!std::holds_alternative<double>(idx_val)) {
                    throw std::runtime_error("STRING_GET: Index must be a number");
                }
                auto str = std::get<std::shared_ptr<NovaString>>(obj);
                size_t idx = static_cast<size_t>(std::get<double>(idx_val));
                if (idx >= str->data.size()) {
                    throw std::runtime_error("String index out of bounds");
                }
                push(std::string(1, str->data[idx]));
                break;
            }

            case OpCode::STRING_SIZE: {
                auto obj = pop();
                if (!std::holds_alternative<std::shared_ptr<NovaString>>(obj)) {
                    throw std::runtime_error("STRING_SIZE: Object is not a string");
                }
                auto str = std::get<std::shared_ptr<NovaString>>(obj);
                push((double)str->data.size());
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
            
            case OpCode::THROW: {
                auto val = pop();
                std::cout << "Error: " << novaStr(val) << "\n";
                running = false;
                break;
            }
            
            case OpCode::TRY_START:
            case OpCode::TRY_END:
            case OpCode::CATCH_START:
                break;
            
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