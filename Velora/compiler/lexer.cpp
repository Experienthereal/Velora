#include "lexer.h"
#include <stdexcept>
#include <cctype>

Lexer::Lexer(const std::string& source_code)
    : source(source_code), pos(0), line(1), column(1), at_line_start(true) {
    keywords = build_keyword_map();
    indent_stack.push(0);
}

char Lexer::current() {
    if (pos >= (int)source.size()) return '\0';
    return source[pos];
}

char Lexer::peek(int offset) {
    int target = pos + offset;
    if (target >= (int)source.size()) return '\0';
    return source[target];
}

void Lexer::advance() {
    if (current() == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    pos++;
}

void Lexer::skip_comment() {
    while (current() != '\n' && current() != '\0') {
        advance();
    }
}

Token Lexer::make_token(TokenType type, const std::string& value) {
    return Token(type, value, line, column);
}

Token Lexer::read_string(char quote) {
    advance();
    std::string result;
    while (current() != quote && current() != '\0') {
        if (current() == '\\') {
            advance();
            switch (current()) {
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
                case 'r':  result += '\r'; break;
                case '\\': result += '\\'; break;
                case '\'': result += '\''; break;
                case '"':  result += '"';  break;
                case '0':  result += '\0'; break;
                default:   result += current(); break;
            }
        } else {
            result += current();
        }
        advance();
    }
    if (current() == quote) advance();
    return make_token(TokenType::STRING, result);
}

Token Lexer::read_fstring() {
    advance();
    char quote = current();
    advance();
    std::string result;
    while (current() != quote && current() != '\0') {
        if (current() == '\\') {
            advance();
            switch (current()) {
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
                case '{':  result += '{';  break;
                case '}':  result += '}';  break;
                default:   result += current(); break;
            }
        } else {
            result += current();
        }
        advance();
    }
    if (current() == quote) advance();
    return make_token(TokenType::FSTRING, result);
}

Token Lexer::read_number() {
    std::string num;
    bool has_dot = false;

    while (std::isdigit(current()) || current() == '.' || current() == '_') {
        if (current() == '.') {
            if (has_dot || peek() == '.') break;
            has_dot = true;
        }
        if (current() != '_') {
            num += current();
        }
        advance();
    }

    return make_token(has_dot ? TokenType::FLOAT : TokenType::INTEGER, num);
}

Token Lexer::read_identifier() {
    std::string id;
    while (std::isalnum(current()) || current() == '_') {
        id += current();
        advance();
    }

    auto it = keywords.find(id);
    if (it != keywords.end()) {
        return make_token(it->second, id);
    }

    return make_token(TokenType::IDENTIFIER, id);
}

Token Lexer::read_color_hex() {
    advance();
    std::string color = "#";
    while (std::isxdigit(current())) {
        color += current();
        advance();
    }
    return make_token(TokenType::COLOR_HEX, color);
}

std::vector<Token> Lexer::handle_indentation() {
    std::vector<Token> tokens;
    int spaces = 0;

    while (current() == ' ') {
        spaces++;
        advance();
    }
    while (current() == '\t') {
        spaces += 4;
        advance();
    }

    if (current() == '\n' || current() == '\r' || current() == '\0') {
        return tokens;
    }
    if (current() == '/' && peek() == '/') {
        return tokens;
    }
    if (current() == '#') {
        return tokens;
    }

    if (spaces > indent_stack.top()) {
        indent_stack.push(spaces);
        tokens.push_back(make_token(TokenType::INDENT, "INDENT"));
    }

    while (spaces < indent_stack.top()) {
        indent_stack.pop();
        tokens.push_back(make_token(TokenType::DEDENT, "DEDENT"));
    }

    return tokens;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < (int)source.size()) {
        if (at_line_start) {
            at_line_start = false;
            auto indent_tokens = handle_indentation();
            for (auto& t : indent_tokens) {
                tokens.push_back(t);
            }
        }

        char c = current();

        if (c == '\0') break;

        if (c == '\n' || c == '\r') {
            if (c == '\r' && peek() == '\n') advance();
            tokens.push_back(make_token(TokenType::NEWLINE, "\\n"));
            advance();
            at_line_start = true;
            continue;
        }

        if (c == ' ' || c == '\t') {
            advance();
            continue;
        }

        if (c == '/' && peek() == '/') {
            skip_comment();
            continue;
        }

        if (c == '#' && !std::isxdigit(peek())) {
            skip_comment();
            continue;
        }

        if (c == 'f' && (peek() == '"' || peek() == '\'')) {
            tokens.push_back(read_fstring());
            continue;
        }

        if (c == '"' || c == '\'') {
            tokens.push_back(read_string(c));
            continue;
        }

        if (std::isdigit(c)) {
            tokens.push_back(read_number());
            continue;
        }

        if (std::isalpha(c) || c == '_') {
            tokens.push_back(read_identifier());
            continue;
        }

        if (c == '#' && std::isxdigit(peek())) {
            tokens.push_back(read_color_hex());
            continue;
        }

        switch (c) {
            case ':':
                advance();
                if (current() == '=') {
                    advance();
                    tokens.push_back(make_token(TokenType::WALRUS, ":="));
                } else if (current() == ':') {
                    advance();
                    tokens.push_back(make_token(TokenType::CONST_DECL, "::"));
                } else {
                    tokens.push_back(make_token(TokenType::COLON, ":"));
                }
                break;
            case '=':
                advance();
                if (current() == '=') {
                    advance();
                    tokens.push_back(make_token(TokenType::EQUAL, "=="));
                } else {
                    tokens.push_back(make_token(TokenType::ASSIGN, "="));
                }
                break;
            case '+':
                advance();
                if (current() == '=') {
                    advance();
                    tokens.push_back(make_token(TokenType::PLUS_ASSIGN, "+="));
                } else {
                    tokens.push_back(make_token(TokenType::PLUS, "+"));
                }
                break;
            case '-':
                advance();
                if (current() == '>') {
                    advance();
                    tokens.push_back(make_token(TokenType::ARROW, "->"));
                } else if (current() == '=') {
                    advance();
                    tokens.push_back(make_token(TokenType::MINUS_ASSIGN, "-="));
                } else {
                    tokens.push_back(make_token(TokenType::MINUS, "-"));
                }
                break;
            case '*':
                advance();
                if (current() == '*') {
                    advance();
                    tokens.push_back(make_token(TokenType::POWER, "**"));
                } else if (current() == '=') {
                    advance();
                    tokens.push_back(make_token(TokenType::STAR_ASSIGN, "*="));
                } else {
                    tokens.push_back(make_token(TokenType::STAR, "*"));
                }
                break;
            case '/':
                advance();
                if (current() == '=') {
                    advance();
                    tokens.push_back(make_token(TokenType::SLASH_ASSIGN, "/="));
                } else {
                    tokens.push_back(make_token(TokenType::SLASH, "/"));
                }
                break;
            case '%':
                advance();
                tokens.push_back(make_token(TokenType::PERCENT, "%"));
                break;
            case '.':
                advance();
                if (current() == '.') {
                    advance();
                    tokens.push_back(make_token(TokenType::DOTDOT, ".."));
                } else {
                    tokens.push_back(make_token(TokenType::DOT, "."));
                }
                break;
            case ',':
                advance();
                tokens.push_back(make_token(TokenType::COMMA, ","));
                break;
            case '@':
                advance();
                tokens.push_back(make_token(TokenType::AT, "@"));
                break;
            case '!':
                advance();
                if (current() == '=') {
                    advance();
                    tokens.push_back(make_token(TokenType::NOT_EQUAL, "!="));
                } else {
                    tokens.push_back(make_token(TokenType::NOT, "!"));
                }
                break;
            case '<':
                advance();
                if (current() == '=') {
                    advance();
                    tokens.push_back(make_token(TokenType::LESS_EQUAL, "<="));
                } else if (current() == '<') {
                    advance();
                    tokens.push_back(make_token(TokenType::SHIFT_LEFT, "<<"));
                } else {
                    tokens.push_back(make_token(TokenType::LESS, "<"));
                }
                break;
            case '>':
                advance();
                if (current() == '=') {
                    advance();
                    tokens.push_back(make_token(TokenType::GREATER_EQUAL, ">="));
                } else if (current() == '>') {
                    advance();
                    tokens.push_back(make_token(TokenType::SHIFT_RIGHT, ">>"));
                } else {
                    tokens.push_back(make_token(TokenType::GREATER, ">"));
                }
                break;
            case '&':
                advance();
                tokens.push_back(make_token(TokenType::AMPERSAND, "&"));
                break;
            case '|':
                advance();
                tokens.push_back(make_token(TokenType::PIPE, "|"));
                break;
            case '^':
                advance();
                tokens.push_back(make_token(TokenType::CARET, "^"));
                break;
            case '~':
                advance();
                tokens.push_back(make_token(TokenType::TILDE, "~"));
                break;
            case '(':
                advance();
                tokens.push_back(make_token(TokenType::LPAREN, "("));
                break;
            case ')':
                advance();
                tokens.push_back(make_token(TokenType::RPAREN, ")"));
                break;
            case '[':
                advance();
                tokens.push_back(make_token(TokenType::LBRACKET, "["));
                break;
            case ']':
                advance();
                tokens.push_back(make_token(TokenType::RBRACKET, "]"));
                break;
            case '{':
                advance();
                tokens.push_back(make_token(TokenType::LBRACE, "{"));
                break;
            case '}':
                advance();
                tokens.push_back(make_token(TokenType::RBRACE, "}"));
                break;
            default:
                advance();
                tokens.push_back(make_token(TokenType::ERROR, std::string(1, c)));
                break;
        }
    }

    while (indent_stack.top() > 0) {
        indent_stack.pop();
        tokens.push_back(make_token(TokenType::DEDENT, "DEDENT"));
    }

    tokens.push_back(make_token(TokenType::END_OF_FILE, ""));
    return tokens;
}
