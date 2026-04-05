#ifndef VELORA_LEXER_H
#define VELORA_LEXER_H

#include "token.h"
#include <vector>
#include <stack>
#include <string>

class Lexer {
    std::string source;
    int pos;
    int line;
    int column;
    std::stack<int> indent_stack;
    std::unordered_map<std::string, TokenType> keywords;
    bool at_line_start;

    char current();
    char peek(int offset = 1);
    void advance();
    void skip_comment();
    Token make_token(TokenType type, const std::string& value);
    Token read_string(char quote);
    Token read_fstring();
    Token read_number();
    Token read_identifier();
    Token read_color_hex();
    std::vector<Token> handle_indentation();

public:
    Lexer(const std::string& source_code);
    std::vector<Token> tokenize();
};

#endif
