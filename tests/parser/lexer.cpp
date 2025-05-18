#include "doctest.h"

#include "mapsc/words.hh"
#include "mapsc/parser/lexer.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Some glyphs should be allowed in identifiers") {
    CHECK(is_allowed_in_identifiers('_'));
    CHECK(is_allowed_in_identifiers('?'));
    CHECK(is_allowed_in_identifiers('!'));
    CHECK(is_allowed_in_identifiers('0'));
}

TEST_CASE("Should lex identifiers correctly") {
    SUBCASE("Semicolon should terminate the identifier") {
        stringstream source{"arsars;"};
        Lexer lexer{&source};

        auto token1 = lexer.get_token();
        auto token2 = lexer.get_token();

        CHECK(token1.token_type == TokenType::identifier);
        CHECK(token1.value == "arsars");

        CHECK(token2.token_type == TokenType::semicolon);
    }

    SUBCASE("Should be able to suffix with ?") {
        stringstream source{"arsars?"};
        Lexer lexer{&source};

        auto token1 = lexer.get_token();
        auto token2 = lexer.get_token();

        CHECK(token1.token_type == TokenType::identifier);
        CHECK(token1.value == "arsars?");
    }

    SUBCASE("Should be able to suffix with !") {
        stringstream source{"id!"};
        Lexer lexer{&source};

        auto token1 = lexer.get_token();
        auto token2 = lexer.get_token();

        CHECK(token1.token_type == TokenType::identifier);
        CHECK(token1.value == "id!");
    }

    SUBCASE("Should handle multiple suffixes") {
        stringstream source{"id__!_?"};
        Lexer lexer{&source};

        auto token1 = lexer.get_token();
        auto token2 = lexer.get_token();

        CHECK(token1.token_type == TokenType::identifier);
        CHECK(token1.value == "id__!_?");
    }
}

TEST_CASE("Should fail on string literal missing closing quote") {
        stringstream source{"\"arsars"};
        Lexer lexer{&source};

        auto token = lexer.get_token();
        CHECK(token.token_type == TokenType::syntax_error);
}