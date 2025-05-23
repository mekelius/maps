#ifndef __PARSER_LAYER_1_HH
#define __PARSER_LAYER_1_HH

#include <array>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "mapsc/source.hh"

#include "mapsc/logging.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/parser/token.hh"
#include "mapsc/parser/lexer.hh"
#include "mapsc/ast/chunk.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/expression.hh"

namespace Maps {

class AST_Store;
class CompilationState;
class PragmaStore;
enum class StatementType;
struct Expression;
struct Statement;

class ParserLayer1 {
public:
    struct Result {
        bool success = true;
        std::optional<RT_Definition*> top_level_definition;
        std::vector<Expression*> unresolved_identifiers;
        std::vector<Expression*> unresolved_type_identifiers;
        std::vector<Expression*> unparsed_termed_expressions;
        std::vector<Expression*> possible_binding_type_declarations;
    };

    ParserLayer1(CompilationState* const state, RT_Scope* scope);

    Result run(std::istream& source_is);
    Result run_eval(std::istream& source_is);

// protected for unit tests
protected:
    void run_parse(std::istream& source_is);
    void prime_tokens();

    // gets the next token from the lexer and stores it in current_token_
    Token get_token();
    const Token& current_token() const;
    const Token& peek() const;

    void update_brace_levels(Token token);
    void reset_to_top_level();
    
    void fail(const std::string& message, SourceLocation location, bool compiler_error = false);
    Expression* fail_expression(const std::string& message, SourceLocation location, 
        bool compiler_error = false);
    Definition* fail_definition(const std::string& message, SourceLocation location, 
        bool compiler_error = false);
    Statement* fail_statement(const std::string& message, SourceLocation location, 
        bool compiler_error = false);
    std::nullopt_t fail_optional(const std::string& message, SourceLocation location, 
        bool compiler_error = false);
    
    void log(const std::string& message, LogLevel loglevel) const;
    void log(const std::string& message, LogLevel loglevel, SourceLocation location) const;

    // ---- IDENTIFIERS -----
    bool identifier_exists(const std::string& name) const;

    void create_identifier(const std::string& name, DefinitionBody body, SourceLocation location);
    void create_identifier(const std::string& name, SourceLocation location);
    std::optional<Definition*> lookup_identifier(const std::string& name);

    // attempts to collapse a single statement block
    bool simplify_single_statement_block(Statement* outer);

    Definition* create_definition(DefinitionBody body, SourceLocation location);
    // creates an expression using ast_, marking the location as the expression_location_stack_
    Statement* create_statement(StatementType statement_type, SourceLocation location);

    // #################################### layer1/pragma.cpp #####################################

    void handle_pragma();

    // ################################## layer1/definition.cpp ###################################

    Chunk parse_top_level_chunk();
    Definition* parse_top_level_let_definition();
    Definition* parse_operator_definition();
    DefinitionBody parse_definition_body();
    Statement* parse_inner_let_definition();
    
    // ################################## layer1/statement.cpp ####################################

    Statement* parse_statement();
    Statement* parse_initial_reserved_word_statement();
    Statement* parse_expression_statement();
    Statement* parse_assignment_statement();
    Statement* parse_return_statement();
    Statement* parse_block_statement();

    // ################################# layer1/expression.cpp ####################################

    Expression* parse_expression();
    Expression* parse_parenthesized_expression();
    Expression* parse_mapping_literal();
    Expression* parse_access_expression();
    Expression* parse_ternary_expression();
    Expression* parse_lambda_expression();

    std::optional<ParameterList> parse_lambda_parameters(RT_Scope* lambda_scope);
    Expression* parse_binding_type_declaration();

    std::optional<const Type*> parse_parameter_type_declaration();

    // ############################## layer1/termed_expression.cpp ################################

    Expression* parse_termed_expression(bool is_tied = false);
    Expression* parse_term(bool is_tied = false);

    // ################################## layer1/terminal.cpp #####################################
    
    Expression* handle_string_literal();
    Expression* handle_numeric_literal();
    Expression* handle_identifier();
    Expression* handle_type_identifier();
    Expression* handle_type_constructor_identifier();
        
    // ------------------------------------ PRIVATE FIELDS ----------------------------------------

    std::unique_ptr<Lexer> lexer_;
    Result result_ = {};

    CompilationState* const compilation_state_;
    RT_Scope* parse_scope_;
    
    AST_Store* const ast_store_;
    PragmaStore* const pragma_store_;
    
    int which_buf_slot_ = 0;
    std::array<Token, 2> token_buf_ = { Token::dummy_token, Token::dummy_token };

    bool force_top_level_eval_ = false;

    // these are automatically incremented and decremented by the get_token()
    unsigned int indent_level_ = 0;
    unsigned int parenthese_level_ = 0;
    unsigned int curly_brace_level_ = 0;
    unsigned int angle_bracket_level_ = 0;
};

} // namespace Maps

#endif