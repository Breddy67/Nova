#include "Parser.h"
#include <sstream>

static std::runtime_error parseErr(const std::string& msg, int line) {
    return std::runtime_error("[Nova ParseError line " + std::to_string(line) + "] " + msg);
}

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), pos_(0) {}

const Token& Parser::current() const { return tokens_[pos_]; }
const Token& Parser::peek(int offset) const {
    size_t idx = pos_ + offset;
    return (idx < tokens_.size()) ? tokens_[idx] : tokens_.back();
}
Token Parser::advance() {
    Token t = tokens_[pos_];
    if (t.type != TokenType::EOF_) ++pos_;
    return t;
}
Token Parser::expect(TokenType type, const std::string& msg) {
    if (current().type != type) {
        std::string err = msg.empty() ? "Expected token but got '" + current().lexeme + "'" : msg;
        throw parseErr(err, current().line);
    }
    return advance();
}
bool Parser::match(TokenType type) {
    if (current().type == type) { advance(); return true; }
    return false;
}
void Parser::skipNewlines() {
    while (current().type == TokenType::NEWLINE) advance();
}
void Parser::expectEndOfStatement() {
    auto tt = current().type;
    if (tt == TokenType::SEMICOLON) {
        advance();
        return;
    }
    if (tt != TokenType::NEWLINE && tt != TokenType::EOF_ && tt != TokenType::RBRACE)
        throw parseErr("Expected end of statement, got '" + current().lexeme + "'", current().line);
}

bool Parser::isTypeKeyword(TokenType tt) {
    return tt == TokenType::NUMBER_TYPE ||
           tt == TokenType::STRING_TYPE ||
           tt == TokenType::BOOL_TYPE ||
           tt == TokenType::NULL_TYPE ||
           tt == TokenType::BUNCH_TYPE ||
           tt == TokenType::MAP_TYPE ||
           tt == TokenType::STACK_TYPE ||
           tt == TokenType::QUEUE_TYPE ||
           tt == TokenType::HEAP_TYPE ||
           tt == TokenType::LINKEDLIST_TYPE;
}

std::unique_ptr<ProgramNode> Parser::parse() {
   // std::cout << "[DEBUG] Parser::parse() started\n";    
    NodeList stmts;
    skipNewlines();
    while (current().type != TokenType::EOF_) {
       // std::cout << "[DEBUG] Parsing statement at line " << current().line << "\n";
        stmts.push_back(parseStatement());
        skipNewlines();
    }
   // std::cout << "[DEBUG] Parser::parse() finished\n";
    return std::make_unique<ProgramNode>(std::move(stmts));
}

NodePtr Parser::parseStatement() {
    skipNewlines();
    auto tt = current().type;
    //std::cout << "[PARSE] parseStatement: token type = " << (int)tt << ", lexeme = " << current().lexeme << "\n";
    
    if (tt == TokenType::PRINT) {
        //std::cout << "[PARSE] Found print statement!\n";
        return parsePrint();
    }
    if (tt == TokenType::DEBUG) {
        advance();
        auto printNode = parsePrint();
        auto* print = dynamic_cast<PrintNode*>(printNode.get());
        if (print) {
            print->isDebug = true;
        }
        return printNode;
    }
    if (isTypeKeyword(tt)) return parseTypeDeclaration();
    if (tt == TokenType::VAR) return parseVarDecl();
    if (tt == TokenType::FUN) return parseFunDecl();
    if (tt == TokenType::IF) return parseIf();
    if (tt == TokenType::LOOP) return parseLoop();
    if (tt == TokenType::WHILE) return parseWhile();
    if (tt == TokenType::RETURN) return parseReturn();
    if (tt == TokenType::TRY) return parseTryCatch();
    if (tt == TokenType::THROW) return parseThrow();
    if (tt == TokenType::BREAK || tt == TokenType::CONTINUE) return parseBreakContinue();

    if (tt == TokenType::IDENT && peek().type == TokenType::EQ)
        return parseAssign();

    return parseExpression();
}

NodePtr Parser::parseTypeDeclaration() {
    std::string typeName = current().lexeme;
    advance();

    std::string name = expect(TokenType::IDENT, "Expected variable name").lexeme;

    NodePtr initializer = nullptr;
    if (current().type == TokenType::EQ) {
        advance();
        if (current().type == TokenType::LBRACE) {
            initializer = parseInitializerList(typeName);
        } else {
            initializer = parseExpression();
        }
    } else if (current().type == TokenType::LBRACE) {
        initializer = parseInitializerList(typeName);
    } else {
        // ── No initializer - create default based on type ──────────────────
        if (typeName == "number" || typeName == "NUMBER_TYPE") {
            initializer = std::make_unique<NumberNode>(0.0);
        } else if (typeName == "string" || typeName == "STRING_TYPE") {
            initializer = std::make_unique<StringNode>("");
        } else if (typeName == "bool" || typeName == "BOOL_TYPE") {
            initializer = std::make_unique<BoolNode>(false);
        } else if (typeName == "null" || typeName == "NULL_TYPE") {
            initializer = std::make_unique<NullNode>();
        } else if (typeName == "bunch" || typeName == "BUNCH_TYPE") {
            initializer = std::make_unique<CallNode>("bunch", NodeList());
        } else if (typeName == "map" || typeName == "MAP_TYPE") {
            initializer = std::make_unique<CallNode>("map", NodeList());
        } else if (typeName == "stack" || typeName == "STACK_TYPE") {
            initializer = std::make_unique<CallNode>("stack", NodeList());
        } else if (typeName == "queue" || typeName == "QUEUE_TYPE") {
            initializer = std::make_unique<CallNode>("queue", NodeList());
        } else if (typeName == "heap" || typeName == "HEAP_TYPE") {
            initializer = std::make_unique<CallNode>("heap", NodeList());
        } else if (typeName == "linkedlist" || typeName == "LINKEDLIST_TYPE") {
            initializer = std::make_unique<CallNode>("linkedlist", NodeList());
        } else {
            initializer = std::make_unique<NullNode>();
        }
    }

    expectEndOfStatement();
    
    return std::make_unique<VarDeclNode>(name, typeName, std::move(initializer));
}
NodePtr Parser::parseVarDecl() {
    advance();
    std::string name = expect(TokenType::IDENT, "Expected variable name").lexeme;
    expect(TokenType::EQ, "Expected '='");
    auto value = parseExpression();
    expectEndOfStatement();
    return std::make_unique<VarDeclNode>(std::move(name), std::move(value));
}

NodePtr Parser::parseAssign() {
    std::string name = advance().lexeme;
    expect(TokenType::EQ, "Expected '='");
    auto value = parseExpression();
    expectEndOfStatement();
    return std::make_unique<AssignNode>(std::move(name), std::move(value));
}

NodePtr Parser::parseFunDecl() {
    advance();
    std::string name = expect(TokenType::IDENT, "Expected function name").lexeme;
    expect(TokenType::LPAREN, "Expected '('");

    std::vector<std::string> params;
    if (current().type != TokenType::RPAREN) {
        params.push_back(expect(TokenType::IDENT, "Expected parameter name").lexeme);
        while (current().type == TokenType::COMMA) {
            advance();
            params.push_back(expect(TokenType::IDENT, "Expected parameter name").lexeme);
        }
    }
    expect(TokenType::RPAREN, "Expected ')'");
    auto body = parseBlock();
    return std::make_unique<FunDeclNode>(std::move(name), std::move(params), std::move(body));
}

NodePtr Parser::parseIf() {
    advance(); // consume 'if'
    //std::cout << "[PARSE] Parsing if condition\n";
    auto condition = parseExpression();
    //std::cout << "[PARSE] If condition parsed, parsing body\n";
    
    skipNewlines();
    auto body = parseBlock();
    
    NodeList elseBody;
    skipNewlines();
    
    while (current().type == TokenType::ELSE) {
        advance();
        if (current().type == TokenType::IF) {
            auto elseIf = parseIf();
            NodeList block;
            block.push_back(std::move(elseIf));
            elseBody = std::move(block);
            break;
        } else {
            skipNewlines();
            elseBody = parseBlock();
            break;
        }
    }
    
    return std::make_unique<IfNode>(std::move(condition), std::move(body), std::move(elseBody));
}

NodePtr Parser::parseLoop() {
    advance(); // consume 'loop'
    expect(TokenType::LPAREN, "Expected '('");

    // ── Parse initialization (i=0) ──────────────────────────────────────────
    NodeList init;
    if (current().type != TokenType::SEMICOLON) {
        auto expr = parseExpression();
        
        // ── Check if it's an assignment (i = 0) ──────────────────────────────
        if (auto* assign = dynamic_cast<AssignNode*>(expr.get())) {
            // Convert to variable declaration: var i = 0
            //std::cout << "[PARSE] Converting assignment to var declaration: " << assign->name << "\n";
            init.push_back(std::make_unique<VarDeclNode>(assign->name, std::move(assign->value)));
        } else {
            init.push_back(std::move(expr));
        }
    }
    expect(TokenType::SEMICOLON, "Expected ';'");

    // ── Parse condition (i<10) ──────────────────────────────────────────────
    NodePtr condition = nullptr;
    if (current().type != TokenType::SEMICOLON) {
        condition = parseExpression();
    }
    expect(TokenType::SEMICOLON, "Expected ';'");

    // ── Parse update (i=i+1) ──────────────────────────────────────────────────
    NodeList update;
    if (current().type != TokenType::RPAREN) {
        update.push_back(parseExpression());
    }
    expect(TokenType::RPAREN, "Expected ')'");

    skipNewlines();
    auto body = parseBlock();
    return std::make_unique<LoopNode>(std::move(init), std::move(condition), std::move(update), std::move(body));
}


NodePtr Parser::parseWhile() {
    advance(); // consume 'while'
    auto condition = parseExpression();
    
    // ── SKIP NEWLINES BEFORE BODY ──────────────────────────────────────────
    skipNewlines();
    
    auto body = parseBlock();
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

NodePtr Parser::parsePrint() {
    //std::cout << "[PARSE] parsePrint() called!\n";
    
    if (current().type != TokenType::PRINT && current().type != TokenType::DEBUG) {
        throw parseErr("Expected 'print' or 'debug'", current().line);
    }
    
    bool isDebug = (current().type == TokenType::DEBUG);
    advance();  // consume 'print' or 'debug'

    expect(TokenType::LPAREN, "Expected '('");

    NodeList args;
    NodePtr end = nullptr;
    NodePtr sep = nullptr;

    // ── Parse arguments ──────────────────────────────────────────────────────
    while (current().type != TokenType::RPAREN && current().type != TokenType::EOF_) {
        //std::cout << "[PARSE] parsePrint: current token = " << current().lexeme << "\n";
        
        // ── CHECK FOR NAMED PARAMETERS ──────────────────────────────────────
        // We need to check if current is "sep" AND next token is "="
        if (current().type == TokenType::SEP && current().lexeme == "sep") {
            // Look ahead to see if next token is "="
            size_t next_pos = pos_ + 1;
            if (next_pos < tokens_.size() && tokens_[next_pos].type == TokenType::EQ) {
                //std::cout << "[PARSE] Found sep= parameter!\n";
                advance(); // consume 'sep'
                advance(); // consume '='
                sep = parseExpression();
                //std::cout << "[PARSE] sep parsed, current token = " << current().lexeme << "\n";
                if (current().type == TokenType::COMMA) {
                    advance();
                }
                continue;
            }
        }
        
        if (current().type == TokenType::END && current().lexeme == "end") {
            size_t next_pos = pos_ + 1;
            if (next_pos < tokens_.size() && tokens_[next_pos].type == TokenType::EQ) {
                //std::cout << "[PARSE] Found end= parameter!\n";
                advance(); // consume 'end'
                advance(); // consume '='
                end = parseExpression();
                //std::cout << "[PARSE] end parsed, current token = " << current().lexeme << "\n";
                if (current().type == TokenType::COMMA) {
                    advance();
                }
                continue;
            }
        }
        
        // ── Normal argument ──────────────────────────────────────────────────
        //std::cout << "[PARSE] parsePrint: parsing normal argument\n";
        args.push_back(parseExpression());
        //std::cout << "[PARSE] parsePrint: after parseExpression, current = " << current().lexeme << "\n";
        if (current().type == TokenType::COMMA) {
            advance();
            //std::cout << "[PARSE] parsePrint: consumed comma\n";
        }
    }

    expect(TokenType::RPAREN, "Expected ')'");
    expectEndOfStatement();

    //std::cout << "[PARSE] parsePrint() returning\n";
    return std::make_unique<PrintNode>(std::move(args), std::move(end), std::move(sep), isDebug);
}

NodePtr Parser::parseReturn() {
    advance();
    auto tt = current().type;
    if (tt == TokenType::NEWLINE || tt == TokenType::RBRACE || tt == TokenType::EOF_) {
        expectEndOfStatement();
        return std::make_unique<ReturnNode>(nullptr);
    }
    auto value = parseExpression();
    expectEndOfStatement();
    return std::make_unique<ReturnNode>(std::move(value));
}

NodePtr Parser::parseTryCatch() {
    advance(); // consume 'try'
    auto tryBlock = parseBlock();

    std::string catchVar;
    NodeList catchBlock;
    NodeList finallyBlock;

    skipNewlines();
    if (current().type == TokenType::CATCH) {
        advance();
        expect(TokenType::LPAREN, "Expected '('");
        catchVar = expect(TokenType::IDENT, "Expected error variable").lexeme;
        std::cout << "[PARSE] catchVar = '" << catchVar << "'\n";  // ← Add debug!
        expect(TokenType::RPAREN, "Expected ')'");
        catchBlock = parseBlock();
    }

    skipNewlines();
    if (current().type == TokenType::FINALLY) {
        advance();
        finallyBlock = parseBlock();
    }

    return std::make_unique<TryCatchNode>(
        std::move(tryBlock), std::move(catchVar),
        std::move(catchBlock), std::move(finallyBlock)
    );
}

NodePtr Parser::parseThrow() {
    advance();
    auto value = parseExpression();
    expectEndOfStatement();
    return std::make_unique<ThrowNode>(std::move(value));
}

NodePtr Parser::parseBreakContinue() {
    bool isBreak = (current().type == TokenType::BREAK);
    advance();
    std::string label;
    if (current().type == TokenType::IDENT) {
        label = current().lexeme;
        advance();
    }
    expectEndOfStatement();
    if (isBreak) {
        return std::make_unique<BreakNode>(std::move(label));
    } else {
        return std::make_unique<ContinueNode>(std::move(label));
    }
}

NodeList Parser::parseBlock() {
    skipNewlines();
    expect(TokenType::LBRACE, "Expected '{'");
    skipNewlines();

    NodeList stmts;
    while (current().type != TokenType::RBRACE && current().type != TokenType::EOF_) {
        stmts.push_back(parseStatement());
        skipNewlines();  // ← Already here!
    }
    expect(TokenType::RBRACE, "Expected '}'");
    return stmts;
}

NodePtr Parser::parseInitializerList(const std::string& typeName) {
    expect(TokenType::LBRACE, "Expected '{'");
    NodeList elements;
    
    if (current().type != TokenType::RBRACE) {
        elements.push_back(parseExpression());
        while (current().type == TokenType::COMMA) {
            advance();
            if (current().type == TokenType::RBRACE) break;
            elements.push_back(parseExpression());
        }
    }
    expect(TokenType::RBRACE, "Expected '}'");
    
    // ── Determine which data structure to create ────────────────────────────
    std::string callName;
    
    // If typeName is specified (from type declaration), use it
    if (!typeName.empty()) {
        callName = typeName;
    } else {
        // Default to "bunch" for bare { } syntax
        callName = "bunch";
    }
    
    // For map, we need special handling
    if (callName == "map" || callName == "MAP_TYPE") {
        // Map initialization: map()
        return std::make_unique<CallNode>("map", NodeList());
    }
    
    // For all other data structures (bunch, stack, queue, heap, linkedlist)
    return std::make_unique<CallNode>(callName, std::move(elements));
}

NodePtr Parser::parseExpression() {
    //std::cout << "[PARSE] parseExpression: current token = " << current().lexeme << "\n";
    
    if (current().type == TokenType::IDENT && peek().type == TokenType::EQ) {
        //std::cout << "[PARSE] Assignment detected\n";
        std::string name = current().lexeme;
        advance();
        advance();
        auto value = parseExpression();
        return std::make_unique<AssignNode>(std::move(name), std::move(value));
    }
    return parseOr();
}

NodePtr Parser::parseOr() {
    //std::cout << "[DEBUG] parseOr() called\n"; 
    auto left = parseAnd();
    while (current().type == TokenType::OR) {
        advance();
        auto right = parseAnd();
        left = std::make_unique<BinOpNode>(std::move(left), "or", std::move(right));
    }
    return left;
}

NodePtr Parser::parseAnd() {
    auto left = parseNot();
    while (current().type == TokenType::AND) {
        advance();
        auto right = parseNot();
        left = std::make_unique<BinOpNode>(std::move(left), "and", std::move(right));
    }
    return left;
}

NodePtr Parser::parseNot() {
    if (current().type == TokenType::NOT) {
        advance();
        auto operand = parseNot();
        return std::make_unique<UnaryOpNode>("not", std::move(operand));
    }
    return parseComparison();
}

NodePtr Parser::parseComparison() {
   // std::cout << "[PARSE] parseComparison: current token = " << current().lexeme << "\n";
    auto left = parseAddition();
    //std::cout << "[PARSE] parseComparison: left parsed, current = " << current().lexeme << "\n";
    
    while (true) {
        std::string op;
        switch (current().type) {
            case TokenType::EQEQ: op = "=="; break;
            case TokenType::NEQ: op = "!="; break;
            case TokenType::LT: op = "<"; break;
            case TokenType::GT: op = ">"; break;
            case TokenType::LTE: op = "<="; break;
            case TokenType::GTE: op = ">="; break;
            default: {
                //std::cout << "[PARSE] parseComparison: no comparison, returning\n";
                return left;
            }
        }
        advance();
        auto right = parseAddition();
        //std::cout << "[PARSE] parseComparison: creating BinOpNode with " << op << "\n";
        left = std::make_unique<BinOpNode>(std::move(left), op, std::move(right));
    }
}

NodePtr Parser::parseAddition() {
    //std::cout << "[DEBUG] parseAddition() called\n"; 
    auto left = parseMultiplication();
    while (current().type == TokenType::PLUS || current().type == TokenType::MINUS) {
        //std::cout << "[DEBUG] parseAddition() found operator: " << current().lexeme << "\n";
        std::string op = (current().type == TokenType::PLUS) ? "+" : "-";
        advance();
        auto right = parseMultiplication();
        //std::cout << "[PARSE] parseAddition: " << op << " with right value\n";
        if (auto* num = dynamic_cast<NumberNode*>(right.get())) {
            //std::cout << "[PARSE] parseAddition: right is number: " << num->value << "\n";
        }
        left = std::make_unique<BinOpNode>(std::move(left), op, std::move(right));
    }
    return left;
}

NodePtr Parser::parseMultiplication() {
   // std::cout << "[DEBUG] parseMultiplication() called, current token: " << current().lexeme << "\n";  // ← MOVED TO TOP!  
    auto left = parsePower();  
    while (true) {
        std::string op;
        switch (current().type) {
            case TokenType::STAR: op = "*"; break;
            case TokenType::SLASH: op = "/"; break;
            case TokenType::PERCENT: op = "%"; break;
            default: return left;
        }
        //std::cout << "[DEBUG] parseMultiplication() found op: " << op << "\n";
        advance();
        auto right = parsePower();
        left = std::make_unique<BinOpNode>(std::move(left), op, std::move(right));
    }
}

NodePtr Parser::parsePower() {
    //std::cout << "[DEBUG] parsePower() called, current token: " << current().lexeme << "\n";
    auto base = parseUnary();
    if (current().type == TokenType::POWER) {
        advance();
        auto exp = parsePower();
        //std::cout << "[PARSE] parseUnary: unary minus on operand\n";
        return std::make_unique<BinOpNode>(std::move(base), "**", std::move(exp));
    }
    return base;
}

NodePtr Parser::parseUnary() {
   // std::cout << "[DEBUG] parseUnary() called, current token: " << current().lexeme << "\n";
    if (current().type == TokenType::MINUS) {
        advance();
        auto operand = parseUnary();
        return std::make_unique<UnaryOpNode>("-", std::move(operand));
    }
    return parseCall();
}

NodePtr Parser::parseCall() {
    auto expr = parsePrimary();

    while (true) {
        if (current().type == TokenType::LPAREN) {
            auto* ident = dynamic_cast<IdentNode*>(expr.get());
            if (!ident)
                throw parseErr("Can only call named functions", current().line);

            std::string name = ident->name;
            
            // ── Skip print - it's handled by parseStatement() ──────────────
            if (name == "print" || name == "debug") {
                // This shouldn't happen, but just in case
                throw parseErr("print should be handled by parseStatement()", current().line);
            }
            
            advance();
            NodeList args;
            if (current().type != TokenType::RPAREN) {
                args.push_back(parseExpression());
                while (current().type == TokenType::COMMA) {
                    advance();
                    args.push_back(parseExpression());
                }
            }
            expect(TokenType::RPAREN, "Expected ')'");
            expr = std::make_unique<CallNode>(std::move(name), std::move(args));
        }
        // ... slicing, indexing ...
        else {
            break;
        }
    }
    return expr;
}
NodePtr Parser::parsePrimary() {
    const Token& tok = current();
    NodePtr expr;

    // ── Parse the base expression ──────────────────────────────────────────
    if (tok.type == TokenType::NUMBER) {
        //std::cout << "[PARSE] Number: " << tok.numVal << "\n";
        advance();
        expr = std::make_unique<NumberNode>(tok.numVal);
    } else if (tok.type == TokenType::STRING) {
        advance();
        expr = std::make_unique<StringNode>(tok.lexeme);
    } else if (tok.type == TokenType::BOOL) {
        advance();
        expr = std::make_unique<BoolNode>(tok.boolVal);
    } else if (tok.type == TokenType::NULL_TYPE || tok.type == TokenType::NULLVAL) {
        advance();
        expr = std::make_unique<NullNode>();
    } else if (tok.type == TokenType::IDENT) {
        advance();
        expr = std::make_unique<IdentNode>(tok.lexeme);
    } else if (tok.type == TokenType::LPAREN) {
        advance();
        expr = parseExpression();
        expect(TokenType::RPAREN, "Expected ')'");
    } else if (tok.type == TokenType::LBRACE) {
        expr = parseInitializerList("");
    } else {
        throw parseErr("Unexpected token '" + tok.lexeme + "' in expression", tok.line);
    }

    // ── Handle postfix operators: [ ] indexing and [:] slicing ────────────
    while (current().type == TokenType::LBRACKET) {
        advance();  // consume '['

        // ── Check if it's a slice (has ':') ──────────────────────────────────
        bool isSlice = false;
        size_t save_pos = pos_;
        
        try {
            if (current().type == TokenType::COLON) {
                isSlice = true;
            } else {
                if (current().type != TokenType::RBRACKET) {
                    parseExpression();
                }
                if (current().type == TokenType::COLON) {
                    isSlice = true;
                }
            }
            pos_ = save_pos;
        } catch (...) {
            pos_ = save_pos;
        }

        if (isSlice) {
            // ── Parse slice: [start:end:step] ──────────────────────────────────
            NodePtr start = nullptr;
            NodePtr end = nullptr;
            NodePtr step = nullptr;

            if (current().type != TokenType::COLON) {
                start = parseExpression();
            }

            expect(TokenType::COLON, "Expected ':'");

            if (current().type != TokenType::COLON && current().type != TokenType::RBRACKET) {
                end = parseExpression();
            }

            if (current().type == TokenType::COLON) {
                advance();
                if (current().type != TokenType::RBRACKET) {
                    step = parseExpression();
                } else {
                    step = std::make_unique<NumberNode>(1.0);
                }
            } else {
                step = std::make_unique<NumberNode>(1.0);
            }

            expect(TokenType::RBRACKET, "Expected ']'");

            if (!start) start = std::make_unique<NumberNode>(0.0);

            expr = std::make_unique<SubpartNode>(
                std::move(expr),
                std::move(start),
                std::move(end),
                std::move(step)
            );
        } else {
            // ── Simple index access: [index] ──────────────────────────────────
            auto index = parseExpression();
            expect(TokenType::RBRACKET, "Expected ']'");
            expr = std::make_unique<IndexAccessNode>(std::move(expr), std::move(index));
        }
    }

    return expr;
}