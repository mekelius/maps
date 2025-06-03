#include "parse_dsir.hh"

#include <string>
#include <string_view>
#include <istream>
#include <sstream>

#include "mapsc/source_location.hh"
#include "mapsc/logging.hh"
#include "mapsc/words.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/ast/identifier.hh"
#include "mapsc/ast/misc_expression.hh"

#include "mapsc/procedures/name_resolution.hh"

using std::optional, std::nullopt;


namespace Maps {

using Log = LogInContext<LogContext::dsir_parser>;

namespace DSIR {
namespace {

constexpr std::array reserved_words {
    "let", "fn", "return",
    "if", "then", "else"
};

struct Token {
    enum class Type {
        eof,
        identifier, type_identifier,
        parenthesis_open, parenthesis_close, comma, semicolon, equals,
        curly_brace_open, curly_brace_close,
        string_literal, number_literal,
        question_mark, colon,
        let, fn, return_t, if_t, then_t, else_t
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
            case Type::equals:
                return "\"=\"";
            case Type::curly_brace_open:
                return "\"{\""; 
            case Type::curly_brace_close:
                return "\"}\"";
            case Type::string_literal:
                return "string literal \"" + value + "\""; 
            case Type::number_literal:
                return "number literal " + value;
            case Type::question_mark:
                return "\"?\""; 
            case Type::colon:
                return "\":\"";
            case Type::let:
                return "\"let\""; 
            case Type::fn:
                return "\"fn\"";
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
                return { Token::Type::question_mark };
            case ':':
                return { Token::Type::colon };
            case '=':
                return { Token::Type::equals };
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

        auto value = buffer.str();
        if (value == "let")
            return { Token::Type::let };
        if (value == "fn")
            return { Token::Type::fn };
        if (value == "return")
            return { Token::Type::return_t };
        if (value == "if")
            return { Token::Type::if_t };
        if (value == "then")
            return { Token::Type::then_t };
        if (value == "else")
            return { Token::Type::else_t };

        return { Token::Type::identifier, value };
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

    ParseResult run() {
        get_token();

        while(!(current_token_.token_type == Token::Type::eof))
            parse_declaration();

        return result_;
    }

    ParseResult run_eval() {
        get_token();

        if (current_token_.token_type == Token::Type::let) {
            parse_declaration();
        } else {
            auto top_level_expression = parse_expression();

        if (!result_.success) {
            Log::error("DSIR eval failed", NO_SOURCE_LOCATION);
            return { false };
        }

        result_.top_level_definition = ast_store_->allocate_definition(
            {"root", top_level_expression, true, NO_SOURCE_LOCATION});
        }

        return result_; 
    }

private:
    void fail(std::string_view message) {
        result_.success = false;
        Log::error(message, current_token_.location);
    }

    Token& get_token() {
        current_token_ = lexer_.get_token();
        return current_token_;
    }

    optional<Definition*> parse_declaration() {
        switch (current_token_.token_type) {
            case Token::Type::let:
                return parse_let_definition();

            case Token::Type::fn:
                return parse_fn_declaration();

            default:
                fail("Unexpected " + current_token_.to_string() + ", expected let or fn");
                return nullopt;
        }
    }
    
    optional<Definition*> parse_fn_declaration() {
        if (current_token_.token_type != Token::Type::fn) {
            fail("Unexpected " + current_token_.to_string() + ", expected fn");
            return nullopt;
        }

        assert(false && "not implemented");
        // identifier
        // (
        // arglist
        // )
        // :
        // return type
        // block

        // if (get_token().token_type != Token::Type::identifier) {
        //     a
        // }
             
        // auto name = current_token_.value;

        // if (get_token().token_type != Token::Type::) {
        //     aseasr
        // }

        // auto body = parse_expression();

        // create definition;
    }

    optional<Definition*> parse_let_definition() {
        if (current_token_.token_type != Token::Type::let) {
            fail("Unexpected " + current_token_.to_string() + ", expected let");
            return nullopt;
        }

        auto location = current_token_.location;

        if (get_token().token_type != Token::Type::identifier) {
            fail("Unexpected " + current_token_.to_string() + ", expected an identifier");
            return nullopt;
        }
             
        auto name = current_token_.value;

        if (get_token().token_type != Token::Type::equals) {
            fail("Unexpected " + current_token_.to_string() + ", expected an \"=\"");
            return nullopt;
        }

        get_token(); // eat the =

        auto body = parse_expression();
        auto definition = ast_store_->allocate_definition({name, true, location}, body);
        if (!result_.definitions.create_identifier(definition)) {
            fail("Couldn't create identifier \"" + name + "\"");
            return nullopt;
        }
        return definition;
    }

    Statement* parse_statement() {
        switch (current_token_.token_type) {
            case Token::Type::string_literal:
            case Token::Type::number_literal:
            case Token::Type::question_mark:
            case Token::Type::identifier:
            case Token::Type::type_identifier:
                return parse_expression_statement(); 
            
            case Token::Type::semicolon: {
                auto statement = create_empty_statement(*ast_store_, current_token_.location);
                get_token();
                return statement;
            }

            case Token::Type::curly_brace_open:
                return parse_block(); 
            case Token::Type::return_t:
                return parse_return_statement(); 
            case Token::Type::if_t:
                return parse_if_statement();

            case Token::Type::colon:
            case Token::Type::then_t:
            case Token::Type::else_t:
            case Token::Type::parenthesis_open:
            case Token::Type::eof:
            case Token::Type::parenthesis_close:
            case Token::Type::comma:
            case Token::Type::equals:
            case Token::Type::curly_brace_close:
            case Token::Type::fn:
            case Token::Type::let:
                assert(false && "inner scopes not implemented");
            default:
                fail("Unexpected " + current_token_.to_string() + ", expected a statement");
                return create_user_error_statement(*ast_store_, current_token_.location);
        }
    }

    Expression* parse_expression() {
        switch (current_token_.token_type) {
            case Token::Type::eof:
                fail("Unexpected eof");
                return create_user_error(*ast_store_, current_token_.location);

            case Token::Type::identifier:
                return parse_identifier();
            case Token::Type::type_identifier:
                return parse_type_identifier();
                
            case Token::Type::question_mark:
                return parse_ternary_expression();

            case Token::Type::string_literal:
                return  parse_string_literal();
            case Token::Type::number_literal:
                return parse_number_literal();

            // case Token::Type::curly_brace_open:
                // return 

            case Token::Type::colon:
            case Token::Type::semicolon:
            case Token::Type::parenthesis_open:
            case Token::Type::parenthesis_close:
            case Token::Type::curly_brace_close:
            case Token::Type::comma:
            default:
                fail("Unexpected " + current_token_.to_string() + ", expected an expression");
                return create_user_error(*ast_store_, current_token_.location);
        }
    }

    Statement* parse_expression_statement() {
        return create_expression_statement(*ast_store_, parse_expression(), current_token_.location);
    }

    DefinitionHeader* parse_let_statement() {
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
        return create_return_statement(*ast_store_, parse_expression(), location);
    }

    Expression* parse_ternary_expression() {
        assert(false && "not implemented");
    }

    Expression* parse_call() {
        assert(false && "not implemented");
        // return create_call(*compilation_state_, , )
    }

    Expression* parse_string_literal() {
        auto expression = create_string_literal(*ast_store_, 
            current_token_.value, current_token_.location);
        get_token();
        return expression;
    }

    Expression* parse_number_literal() {
        auto expression = create_numeric_literal(*ast_store_, 
            current_token_.value, current_token_.location);
        get_token();
        return expression;
    }

    Expression* parse_identifier() {
        auto expression = create_identifier(*ast_store_, &result_.definitions,
            current_token_.value, current_token_.location);
        result_.unresolved_identifiers.push_back(expression);

        get_token();
        return expression;
    }

    Expression* parse_type_identifier() {
        auto expression = create_identifier(*ast_store_, &result_.definitions,
            current_token_.value, current_token_.location);
        result_.unresolved_identifiers.push_back(expression);

        get_token();
        return expression;
    }

    ParseResult result_;
    Token current_token_;
    CompilationState* compilation_state_;
    AST_Store* ast_store_;
    Lexer lexer_;
};

} // anonymous namespace

ParseResult eval_parse_dsir(CompilationState& state, std::istream& source) {
    ParseResult result = DSIR::Parser{&state, &source}.run_eval();

    if (!result.success)
        return result;

    if (result.top_level_definition) {
        if (!resolve_identifiers(state, result.definitions, result.unresolved_identifiers)) {
            Log::compiler_error("Name resolution failed", NO_SOURCE_LOCATION);
            return { false };
        }
    } else {
        if (!resolve_identifiers(state, result.definitions, result.unresolved_identifiers)) {
            Log::compiler_error("Name resolution failed", NO_SOURCE_LOCATION);
            return { false };
        }
    }

    return result;
}

ParseResult parse_dsir(CompilationState& state, std::istream& source) {
    ParseResult result = DSIR::Parser{&state, &source}.run();

    if (!result.success)
        return result;

    if (result.top_level_definition) {
        if (!resolve_identifiers(state, result.definitions, result.unresolved_identifiers)) {
            Log::compiler_error("Name resolution failed", NO_SOURCE_LOCATION);
            return { false };
        }
    } else {
        if (!resolve_identifiers(state, result.definitions, result.unresolved_identifiers)) {
            Log::compiler_error("Name resolution failed", NO_SOURCE_LOCATION);
            return { false };
        }
    }

    return result;
}

} // namespace DSIR
} // namespace Maps