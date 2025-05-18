#include "parse_dsir.hh"

#include <string>
#include <string_view>
#include <istream>
#include <sstream>

#include "mapsc/source.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/logging.hh"
#include "mapsc/words.hh"

using std::optional, std::nullopt;

namespace Maps {

using Log = LogInContext<LogContext::dsir_parser>;

namespace DSIR {

constexpr std::array<std::string_view, 5> reserved_words {
    "let", "return",
    "if", "then", "else"
};

struct Token {
    enum class Type {
        eof,
        identifier, type_identifier,
        parenthesis_open, parenthesis_close, comma, semicolon, assignment_operator,
        curly_brace_open, curly_brace_close,
        string_literal, number_literal,
        ternary_operator, ternary_else,
        let, return_t, if_t, then_t, else_t
    };

    Type token_type;
    std::string value = "";
    SourceLocation location = NO_SOURCE_LOCATION;

    std::string to_string() {
        switch (token_type) {
            case Type::eof:
                return "eof";
            case Type::identifier:
                return "identifier \"" + value + "\"";
            case Type::type_identifier:
                return "type identifier \"" + value + "\"";
            case Type::parenthesis_open:
                return "\"(\"";
            case Type::parenthesis_close:
                return "\")\"";
            case Type::comma:
                return "\",\""; 
            case Type::semicolon:
                return "\";\""; 
            case Type::assignment_operator:
                return "";
            case Type::curly_brace_open:
                return "\"{\""; 
            case Type::curly_brace_close:
                return "\"}\"";
            case Type::string_literal:
                return "string literal \"" + value + "\""; 
            case Type::number_literal:
                return "number literal " + value;
            case Type::ternary_operator:
                return "\"?\""; 
            case Type::ternary_else:
                return "\":\"";
            case Type::let:
                return "\"let\""; 
            case Type::return_t:
                return "\"return\""; 
            case Type::if_t:
                return "\"if\""; 
            case Type::then_t:
                return "\"then\""; 
            case Type::else_t:
                return "\"else\"";
        }
    }
};

class Lexer {
public:
    Lexer(std::istream* source): source_(source) {}

    Token get_token() {
        if (source_->eof())
            return {Token::Type::eof};

        while (is_ignored_char(read_char()));
        
        switch (current_char_) {
            case '/':
                if (read_char() != '/')
                    assert(false && "not implemented");
                read_comment();
                return get_token();

            case ',':
                return { Token::Type::comma };
            case '(':
                return { Token::Type::parenthesis_open };
            case ')':
                return { Token::Type::parenthesis_close };
            case '?':
                return { Token::Type::ternary_operator };
            case ':':
                return { Token::Type::ternary_operator };
            case '=':
                return { Token::Type::assignment_operator };
            case ';':
                while (read_char() == ';');
                return { Token::Type::semicolon };

            case '"':
                return read_string_literal();

            default:
                if (std::isdigit(current_char_))
                    return read_number_literal();

                if (std::isalpha(current_char_) || current_char_ == '_')
                    return read_identifier();

                Log::error(
                    "Unexpected character: \"" + std::string{current_char_} + "\" in dsir",
                    NO_SOURCE_LOCATION);
                return { Token::Type::eof };
        }
    }

    Token read_string_literal() {
        std::stringbuf buffer{};

        while (read_char() != '\"' && !source_->eof())
            buffer.sputc(current_char_);

        return { Token::Type::string_literal, buffer.str() };
    }

    Token read_number_literal() {
        std::stringbuf buffer{};

        do buffer.sputc(current_char_);
        while (std::isdigit(read_char()) && !source_->eof());

        return { Token::Type::number_literal, buffer.str() };
    }

    Token read_identifier() {

        std::stringbuf buffer{};

        do buffer.sputc(current_char_);
        while (is_allowed_in_identifiers(read_char()) && !source_->eof());

        return { Token::Type::identifier, "" };
    }

    void read_comment() {
        while (read_char() != '\n' && !source_->eof());
    }

private:
    char read_char() {
        source_->get(current_char_);
        return current_char_;
    }

    bool is_ignored_char(char ch) {
        return (
            ch == ' '  ||
            ch == '\n' ||
            ch == '\r'
        );
    }

    std::istream* source_;
    char current_char_ = '\00';
};
 
class Parser {
public:
    Parser(CompilationState* compilation_state, std::istream* source)
    :compilation_state_(compilation_state), 
     ast_store_(compilation_state->ast_store_.get()),
     lexer_(source) {
    }

    bool run() {
        get_token();
        
        return compilation_state_->is_valid;
    }

    optional<Callable*> run_eval() {
        get_token();

        auto root_expression = parse_expression();

        if (!compilation_state_->is_valid) {
            Log::error("DSIR eval failed", NO_SOURCE_LOCATION);
            return nullopt;
        }

        auto root_callable = ast_store_->allocate_callable(
            {"root", root_expression, NO_SOURCE_LOCATION});


        if (!compilation_state_->set_entry_point(root_callable)) {
            Log::error("DSIR eval failed: couldn't set entry point", NO_SOURCE_LOCATION);
            compilation_state_->declare_invalid();
            return nullopt;
        }

        return root_callable; 
    }

private:
    void fail(std::string_view message) {
        compilation_state_->declare_invalid();
        Log::error(message, current_token_.location);
    }

    Token& get_token() {
        current_token_ = lexer_.get_token();
        return current_token_;
    }

    Callable* parse_declaration() {
        assert(false && "not implemented");
        // return;
    }

    Statement* parse_statement() {
        switch (current_token_.token_type) {
            case Token::Type::let:
                assert(false && "not implemented");

            case Token::Type::string_literal:
            case Token::Type::number_literal:
            case Token::Type::ternary_operator:
            case Token::Type::identifier:
            case Token::Type::type_identifier:
                return parse_expression_statement(); 
            
            case Token::Type::semicolon: {
                auto statement = Statement::empty(*ast_store_, current_token_.location);
                get_token();
                return statement;
            }

            case Token::Type::curly_brace_open:
                return parse_block(); 
            case Token::Type::return_t:
                return parse_return_statement(); 
            case Token::Type::if_t:
                return parse_if_statement();

            case Token::Type::ternary_else:
            case Token::Type::then_t:
            case Token::Type::else_t:
            case Token::Type::parenthesis_open:
            case Token::Type::eof:
            case Token::Type::parenthesis_close:
            case Token::Type::comma:
            case Token::Type::assignment_operator:
            case Token::Type::curly_brace_close:
            default:
                fail("Unexpected " + current_token_.to_string() + ", expected a statement");
                return Statement::syntax_error(*ast_store_, current_token_.location);
        }
    }

    Expression* parse_expression() {
        switch (current_token_.token_type) {
            case Token::Type::eof:
                fail("Unexpected eof");
                return Expression::syntax_error(*ast_store_, current_token_.location);

            case Token::Type::identifier:
                return parse_identifier();
            case Token::Type::type_identifier:
                return parse_type_identifier();
                
            case Token::Type::ternary_operator:
                return parse_ternary_expression();

            case Token::Type::string_literal:
                return  parse_string_literal();
            case Token::Type::number_literal:
                return parse_number_literal();

            // case Token::Type::curly_brace_open:
                // return 

            case Token::Type::ternary_else:
            case Token::Type::semicolon:
            case Token::Type::parenthesis_open:
            case Token::Type::parenthesis_close:
            case Token::Type::curly_brace_close:
            case Token::Type::comma:
            default:
                fail("Unexpected " + current_token_.to_string() + ", expected an expression");
                return Expression::syntax_error(*ast_store_, current_token_.location);
        }
    }

    Statement* parse_expression_statement() {
        return Statement::expression(*ast_store_, parse_expression(), current_token_.location);
    }

    Callable* parse_let_statement() {
        assert(false && "not implemented");
    }

    Statement* parse_block() {
        assert(false && "not implemented");
    }

    Statement* parse_if_statement() {
        assert(false && "not implemented");
    }

    Statement* parse_return_statement() {
        auto location = current_token_.location;
        get_token(); // eat the "return"
        return Statement::return_(*ast_store_, parse_expression(), location);
    }

    Expression* parse_ternary_expression() {
        assert(false && "not implemented");
    }

    Expression* parse_call() {
        assert(false && "not implemented");
        // return Expression::call(*compilation_state_, , )
    }

    Expression* parse_string_literal() {
        auto expression = Expression::string_literal(*ast_store_, 
            current_token_.value, current_token_.location);
        get_token();
        return expression;
    }

    Expression* parse_number_literal() {
        auto expression = Expression::numeric_literal(*ast_store_, 
            current_token_.value, current_token_.location);
        get_token();
        return expression;
    }

    Expression* parse_identifier() {
        auto expression = Expression::identifier(*compilation_state_,
            current_token_.value, current_token_.location);

        get_token();
        return expression;
    }

    Expression* parse_type_identifier() {
        auto expression = Expression::identifier(*compilation_state_,
            current_token_.value, current_token_.location);

        get_token();
        return expression;
    }

    Token current_token_;
    CompilationState* compilation_state_;
    AST_Store* ast_store_;
    Lexer lexer_;
};

} // namespace DSIR

optional<Callable*> eval_parse_dsir(CompilationState& state, std::istream& source) {
    if (!DSIR::Parser{&state, &source}.run_eval())
        return nullopt;

    // resolve names

    return state.entry_point_;
}

bool parse_dsir(CompilationState& state, std::istream& source) {
    if (!DSIR::Parser{&state, &source}.run())
        return false;
    // resolve names

    return state.is_valid;
}

} // namespace Maps