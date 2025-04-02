#include "lexer.hh"
// ----- Lexer -----

enum class Token: int {
    eof,
    identifier,
    number,
    oper
};

// -----------------

void mainloop() {
    // Token current_token = get_token();

    // while (current_token != token::eof) {
    //     switch (current_token) {
    //         case token::eof:
    //         return;
    //     }
    // }
}