#pragma once
#include "../AST.h"
#include "BytecodeProgram.h"
#include <vector>
#include <stack>
#include "../Environment.h" 

class BytecodeCompiler : public Visitor {
public:
    BytecodeCompiler();
    BytecodeProgram compile(ProgramNode& program);
    
    void visitNumber(NumberNode& node) override;
    void visitString(StringNode& node) override;
    void visitBool(BoolNode& node) override;
    void visitNull(NullNode& node) override;
    void visitBinOp(BinOpNode& node) override;
    void visitUnaryOp(UnaryOpNode& node) override;
    void visitIdent(IdentNode& node) override;
    void visitCall(CallNode& node) override;
    void visitIndexAccess(IndexAccessNode& node) override;
//    void visitMethodCall(MethodCallNode& node) override;
    void visitSubpart(SubpartNode& node) override;
    void visitProgram(ProgramNode& node) override;
    void visitVarDecl(VarDeclNode& node) override;
    void visitAssign(AssignNode& node) override;
    void visitFunDecl(FunDeclNode& node) override;
    void visitIf(IfNode& node) override;
    void visitLoop(LoopNode& node) override;
    void visitWhile(WhileNode& node) override;
    void visitPrint(PrintNode& node) override;
    void visitReturn(ReturnNode& node) override;
    void visitTryCatch(TryCatchNode& node) override;
    void visitThrow(ThrowNode& node) override;
    void visitBreak(BreakNode& node) override;
    void visitContinue(ContinueNode& node) override;
    
private:
    BytecodeProgram program;
    std::stack<size_t> break_stack;
    std::stack<size_t> continue_stack;
    std::stack<size_t> loop_start_stack;
    std::vector<size_t> break_positions; 
    std::vector<size_t> continue_positions;
    std::unordered_map<std::string,size_t> functionAddresses;
    std::vector<std::shared_ptr<FunDeclNode>> functions;
    
    void compileExpression(Node& node);
    void compileStatement(Node& node);
    void compileBlock(const NodeList& block);
    void compileInitializerList(const NodeList& elements);
    
    void emitMethodCall(const std::string& method, const NodeList& args, const std::string& objectType);
    void compileBunchOperation(const std::string& method, const NodeList& args);
    void compileMapOperation(const std::string& method, const NodeList& args);
    void compileStackOperation(const std::string& method, const NodeList& args);
    void compileQueueOperation(const std::string& method, const NodeList& args);
    void compileHeapOperation(const std::string& method, const NodeList& args);
    void compileListOperation(const std::string& method, const NodeList& args);
    void compileTreeOperation(const std::string& method, const NodeList& args);
    void compileGraphOperation(const std::string& method, const NodeList& args);
};