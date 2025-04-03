#ifndef __PARSER_HH
#define __PARSER_HH

#include <istream>
#include <memory>
#include <array>

#include "lexer.hh"
#include "ir_gen.hh"

// First attempt at a parser. Parses tokens directly into the llvm context
class DirectParser {
  public:
    DirectParser(StreamingLexer* lexer, IRGenHelper* ir_gen);

    void run();
  private:
    StreamingLexer* lexer_;
    IRGenHelper* ir_gen_;   
    Token current_token_;
};

#endif