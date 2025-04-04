#ifndef __PARSER_HH
#define __PARSER_HH

#include <istream>
#include <memory>
#include <array>
#include <ostream>
#include <vector>
#include <unordered_map>

#include "syntax_error.hh"
#include "lexer.hh"
// #include "ir_gen.hh"

namespace AST {

// struct Type {

// };

// struct Mapping: public Type {
    // Type* domain;
    // Type* codomain;
// };

// struct ConcreteType: public Type {};
// struct ParameterizedType: public Type {
//     std::vector<TypeExpression> 
// };

// struct Function: public Mapping {};

enum class IdentifierClass {
    variable,
    builtin
    // type,
};

struct Identifier {
    // IdentifierClass identifier_class;
    // std::unique_ptr<Expression> expression;
};

struct Expression {    
    // std::vector<Expression*> sub_expressions;
};

struct Value: public Expression {};
struct NumberValue: public Value {
    double value;
};
// struct Value: public Expression {};
// struct Constant: public Value {};
// struct Variable: public Value {};

// struct Call: public Expression {};

} // namespace AST

// struct Block: public Expression {};
// struct TypeExpression: public Expression {};
  
// First attempt at a parser. Parses tokens directly into the llvm context
class Parser {
  public:
    Parser(StreamingLexer* lexer, std::ostream* error_stream);
    
    void run();
  private:
    // gets the next token from the lexer and stores it in current_token_
    Token get_token();
    // Token expect_token(predicate, error_message);
    void syntax_error(const std::string& message);
    bool identifier_exists(const std::string& identifier);
    void create_identifier(const std::string& identifier, AST::Expression* expression);

    AST::Expression* parse_expression();
    void parse_let_statement();
    
    StreamingLexer* lexer_;
    std::ostream* errs_;
    
    Token current_token_;
    std::unordered_map<std::string, AST::Expression*> identifiers_;
    std::vector<std::unique_ptr<AST::Expression>> expressions_;
};

#endif