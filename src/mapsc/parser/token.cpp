#include "token.hh"

#include <cassert>
#include <optional>

#include "mapsc/logging.hh"

using std::optional, std::nullopt;

namespace Maps {

Token::Token(TokenType token_type, const std::string& value, SourceLocation location)
:token_type(token_type), value(value), location(location) {}

Token::Token(TokenType token_type, SourceLocation location)
:Token(token_type, "", location) {}

LogStream& Token::log_self_to(LogStream& logstream) const {
    switch (token_type) {
        case TokenType::eof:
            return logstream << "EOF";
        case TokenType::dummy:
            return logstream << "DUMMY TOKEN";

        case TokenType::identifier:
            return logstream << "identifier \"" << string_value() << '"';
        case TokenType::type_identifier:
            return logstream << "type identifier \"" + string_value() << '"';
        case TokenType::operator_t:
            return logstream << "operator \"" << string_value() << '"';
        case TokenType::arrow_operator:
            return logstream << "arrow operator \"" << string_value() << '"';    
    
        case TokenType::number:
            return logstream << "numeric literal " << string_value();
        case TokenType::string_literal:
            return logstream << "string literal \"" << string_value() << "\"";

        case TokenType::indent_block_start:
            return logstream << "indent block start";
        case TokenType::indent_block_end:
            return logstream << "indent block end";
        case TokenType::indent_error_fatal:
            return logstream << "indent error fatal";

        case TokenType::curly_brace_open:
            return logstream << "{";
        case TokenType::curly_brace_close:
            return logstream << "}";
        case TokenType::parenthesis_open:
            return logstream << "(";
        case TokenType::parenthesis_close:
            return logstream << ")";
        case TokenType::bracket_open:
            return logstream << "[";
        case TokenType::bracket_close:
            return logstream << "]";
        case TokenType::semicolon:
            return logstream << ";";
        case TokenType::question_mark:
            return logstream << "?";
        case TokenType::colon:
            return logstream << ":";
        case TokenType::double_colon:
            return logstream << "::";
        case TokenType::comma:
            return logstream << ",";
        case TokenType::lambda:
            return logstream << "\\";
        case TokenType::underscore:
            return logstream << "_";
            
        case TokenType::if_t:
            return logstream << "if";
        case TokenType::then:
            return logstream << "then";
        case TokenType::else_t:
            return logstream << "else";
        case TokenType::while_t:
            return logstream << "while";
        case TokenType::for_t:
            return logstream << "for";
        case TokenType::do_t:
            return logstream << "do";
        case TokenType::guard:
            return logstream << "guard";
        case TokenType::switch_t:
            return logstream << "switch";
        case TokenType::case_t:
            return logstream << "case";
        case TokenType::yield_t:
            return logstream << "yield";
        case TokenType::let:
            return logstream << "let";
        case TokenType::return_t:
            return logstream << "return";
        case TokenType::operator_rwt:
            return logstream << "operator";         
        case TokenType::unary:
            return logstream << "unary";     
        case TokenType::binary:
            return logstream << "binary";             
        case TokenType::prefix:
            return logstream << "prefix";             
        case TokenType::postfix:
            return logstream << "postfix";  

        case TokenType::tie:
            return logstream << "tie";

        case TokenType::pragma:
            return logstream << "pragma: " << string_value();

        case TokenType::syntax_error:
            return logstream << "syntax error: " << string_value();
    }
}

bool is_assignment_operator(const Token& token) {
    if (token.token_type != TokenType::operator_t)
        return false;
    
    return (
        token.string_value() == "="    //||
        // token.value == "+="     ||
        // token.value == "-="     ||
        // token.value == "++"     ||
        // token.value == "--"     ||
        // token.value == "?="     ||
    );
}

bool is_statement_separator(const Token& token) {
    switch (token.token_type) {
        case TokenType::semicolon:
        case TokenType::eof:
        case TokenType::indent_block_end:
        case TokenType::indent_error_fatal:
        case TokenType::curly_brace_close:
        case TokenType::bracket_close:
        case TokenType::parenthesis_close:
        case TokenType::if_t:
        case TokenType::then:
        case TokenType::else_t:
        case TokenType::return_t:
        case TokenType::for_t:
        case TokenType::while_t:
        case TokenType::switch_t:
        case TokenType::do_t:
            return true;
                
        default: 
            return false;
    }
}

bool is_expression_ender(const Token& token) {
    switch (token.token_type) {
        case TokenType::semicolon:
        case TokenType::eof:
        case TokenType::indent_block_end:
        case TokenType::indent_error_fatal:
        case TokenType::curly_brace_close:
        case TokenType::bracket_close:
        case TokenType::if_t:
        case TokenType::then:
        case TokenType::else_t:
        case TokenType::return_t:
        case TokenType::for_t:
        case TokenType::while_t:
        case TokenType::do_t:
        case TokenType::switch_t:
            return true;

        default: 
            return false;
    }
}

bool is_access_operator(const Token& token) {
    switch (token.token_type) {
        case TokenType::operator_t:
            return (token.string_value() == ".");

        case TokenType::parenthesis_open:
        case TokenType::curly_brace_open:
        case TokenType::bracket_open:
        case TokenType::double_colon:
            return true;
        
        default:
            return false;
    }
}

bool is_block_starter(const Token& token) {
    switch (token.token_type) {
        case TokenType::indent_block_start:
        case TokenType::curly_brace_open:
        case TokenType::then:
        case TokenType::else_t:
            return true;

        default:
            return false;
    }
}

// these can be FOLLOWED by a tie
bool is_right_tieable_token(const Token& token) {
    // ? should type identifiers be tieable?
    switch (token.token_type) {
        case TokenType::operator_t:
        case TokenType::identifier:
        case TokenType::number:
        case TokenType::double_colon:
        case TokenType::string_literal:
        case TokenType::type_identifier:
        case TokenType::arrow_operator:
            return true;

        default:
            return false;
    }
}

// these are allowed to appear in termed expressions
bool is_term_token(const Token& token) {
    switch (token.token_type) {
        case TokenType::string_literal:
        case TokenType::number:
        case TokenType::parenthesis_open:
        case TokenType::bracket_open:
        case TokenType::curly_brace_open:
        case TokenType::identifier:
        case TokenType::type_identifier:
        case TokenType::indent_block_start:
        case TokenType::lambda:
        case TokenType::operator_t:
        case TokenType::arrow_operator:
        case TokenType::tie:
        case TokenType::colon:
        case TokenType::double_colon:
            return true;

        default:
            return false;
    }
}

bool is_condition_ender(const Token& token) {
    switch (token.token_type) {
        case TokenType::then:
        case TokenType::do_t:
            return true;
        default:
            return false;
    }
}


const Token Token::dummy_token{TokenType::dummy, NO_SOURCE_LOCATION};

namespace {

std::map<std::string, TokenType> reserved_words_map{
        { "if",         TokenType::if_t         },
        { "then",       TokenType::then         },
        { "else",       TokenType::else_t       },
        { "while",      TokenType::while_t      },
        { "for",        TokenType::for_t        },
        { "do",         TokenType::do_t         },
        { "guard",      TokenType::guard        },
        { "switch",     TokenType::switch_t     },
        { "case",       TokenType::case_t       },
        { "yield",      TokenType::yield_t      },
        { "let",        TokenType::let          },
        { "return",     TokenType::return_t     },
        { "operator",   TokenType::operator_rwt },
        { "unary",      TokenType::unary        },
        { "binary",     TokenType::binary       },
        { "prefix",     TokenType::prefix       },
        { "postfix",    TokenType::postfix      },
};
    
} // namespace

optional<TokenType> lookup_reserved_word_token_type(const std::string& str) {
    auto it = reserved_words_map.find(str);
    if (it == reserved_words_map.end())
        return nullopt;

    return it->second;
}

} // namespace Maps