#ifndef VELORA_TOKEN_H
#define VELORA_TOKEN_H

#include <string>
#include <unordered_map>

enum class TokenType {
    // Literals
    INTEGER,
    FLOAT,
    STRING,
    FSTRING,
    RAWSTRING,
    BOOL_TRUE,
    BOOL_FALSE,
    IDENTIFIER,
    COLOR_HEX,

    // Operators
    ASSIGN,         // =
    WALRUS,         // :=
    CONST_DECL,     // ::
    COLON,          // :
    ARROW,          // ->
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    POWER,          // **
    DOT,
    DOTDOT,         // ..
    COMMA,
    AT,             // @

    // Comparison
    EQUAL,          // ==
    NOT_EQUAL,      // !=
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,

    // Logical
    AND,
    OR,
    NOT,

    // Bitwise
    AMPERSAND,      // &
    PIPE,           // |
    CARET,          // ^
    TILDE,          // ~
    SHIFT_LEFT,     // <<
    SHIFT_RIGHT,    // >>

    // Compound assignment
    PLUS_ASSIGN,    // +=
    MINUS_ASSIGN,   // -=
    STAR_ASSIGN,    // *=
    SLASH_ASSIGN,   // /=

    // Delimiters
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,

    // Block control
    INDENT,
    DEDENT,
    NEWLINE,

    // Keywords
    KW_FUNC,
    KW_STRUCT,
    KW_CLASS,
    KW_TRAIT,
    KW_IF,
    KW_ELIF,
    KW_ELSE,
    KW_FOR,
    KW_WHILE,
    KW_LOOP,
    KW_IN,
    KW_MATCH,
    KW_DEFAULT,
    KW_RETURN,
    KW_BREAK,
    KW_CONTINUE,
    KW_PASS,
    KW_IMPORT,
    KW_FROM,
    KW_EXPORT,
    KW_MODULE,
    KW_CREATE,
    KW_AS,
    KW_WITH,
    KW_TRY,
    KW_CATCH,
    KW_MAIN,
    KW_VELORAGAME,
    KW_GAME_LOOP,
    KW_ASYNC,
    KW_AWAIT,
    KW_REF,

    // Type keywords
    KW_INT,
    KW_INT8,
    KW_INT16,
    KW_INT32,
    KW_INT64,
    KW_UINT8,
    KW_UINT16,
    KW_UINT32,
    KW_UINT64,
    KW_FLOAT,
    KW_DOUBLE,
    KW_BOOL,
    KW_CHAR,
    KW_STRING,
    KW_RAWSTRING,
    KW_LIST,
    KW_ARRAY,
    KW_DICT,
    KW_SET,
    KW_TUPLE,
    KW_OPTIONAL,
    KW_VECTOR2,
    KW_VECTOR3,
    KW_VECTOR4,
    KW_MATRIX3,
    KW_MATRIX4,
    KW_QUATERNION,
    KW_COLOR,

    // Special
    END_OF_FILE,
    ERROR
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    Token() : type(TokenType::ERROR), value(""), line(0), column(0) {}
    Token(TokenType t, const std::string& v, int ln, int col)
        : type(t), value(v), line(ln), column(col) {}
};

inline std::unordered_map<std::string, TokenType> build_keyword_map() {
    return {
        {"func",        TokenType::KW_FUNC},
        {"struct",      TokenType::KW_STRUCT},
        {"class",       TokenType::KW_CLASS},
        {"trait",       TokenType::KW_TRAIT},
        {"if",          TokenType::KW_IF},
        {"elif",        TokenType::KW_ELIF},
        {"else",        TokenType::KW_ELSE},
        {"for",         TokenType::KW_FOR},
        {"while",       TokenType::KW_WHILE},
        {"loop",        TokenType::KW_LOOP},
        {"in",          TokenType::KW_IN},
        {"match",       TokenType::KW_MATCH},
        {"default",     TokenType::KW_DEFAULT},
        {"return",      TokenType::KW_RETURN},
        {"break",       TokenType::KW_BREAK},
        {"continue",    TokenType::KW_CONTINUE},
        {"pass",        TokenType::KW_PASS},
        {"import",      TokenType::KW_IMPORT},
        {"from",        TokenType::KW_FROM},
        {"export",      TokenType::KW_EXPORT},
        {"module",      TokenType::KW_MODULE},
        {"create",      TokenType::KW_CREATE},
        {"as",          TokenType::KW_AS},
        {"with",        TokenType::KW_WITH},
        {"try",         TokenType::KW_TRY},
        {"catch",       TokenType::KW_CATCH},
        {"main",        TokenType::KW_MAIN},
        {"Veloragame",  TokenType::KW_VELORAGAME},
        {"game_loop",   TokenType::KW_GAME_LOOP},
        {"async",       TokenType::KW_ASYNC},
        {"await",       TokenType::KW_AWAIT},
        {"ref",         TokenType::KW_REF},
        {"true",        TokenType::BOOL_TRUE},
        {"false",       TokenType::BOOL_FALSE},
        {"and",         TokenType::AND},
        {"or",          TokenType::OR},
        {"not",         TokenType::NOT},
        {"int",         TokenType::KW_INT},
        {"int8",        TokenType::KW_INT8},
        {"int16",       TokenType::KW_INT16},
        {"int32",       TokenType::KW_INT32},
        {"int64",       TokenType::KW_INT64},
        {"uint8",       TokenType::KW_UINT8},
        {"uint16",      TokenType::KW_UINT16},
        {"uint32",      TokenType::KW_UINT32},
        {"uint64",      TokenType::KW_UINT64},
        {"float",       TokenType::KW_FLOAT},
        {"double",      TokenType::KW_DOUBLE},
        {"bool",        TokenType::KW_BOOL},
        {"char",        TokenType::KW_CHAR},
        {"string",      TokenType::KW_STRING},
        {"rawstring",   TokenType::KW_RAWSTRING},
        {"list",        TokenType::KW_LIST},
        {"array",       TokenType::KW_ARRAY},
        {"dict",        TokenType::KW_DICT},
        {"set",         TokenType::KW_SET},
        {"tuple",       TokenType::KW_TUPLE},
        {"optional",    TokenType::KW_OPTIONAL},
        {"Vector2",     TokenType::KW_VECTOR2},
        {"Vector3",     TokenType::KW_VECTOR3},
        {"Vector4",     TokenType::KW_VECTOR4},
        {"Matrix3",     TokenType::KW_MATRIX3},
        {"Matrix4",     TokenType::KW_MATRIX4},
        {"Quaternion",  TokenType::KW_QUATERNION},
        {"Color",       TokenType::KW_COLOR},
    };
}

#endif
