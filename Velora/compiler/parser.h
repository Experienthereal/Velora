#ifndef VELORA_PARSER_H
#define VELORA_PARSER_H

#include "token.h"
#include "ast.h"
#include <vector>

class Parser {
    std::vector<Token> tokens;
    int pos;

    Token current();
    Token peek(int offset = 1);
    Token eat(TokenType expected);
    Token advance();
    bool check(TokenType type);
    bool match(TokenType type);
    void skip_newlines();
    void expect_newline();

    // Top-level
    ASTPtr parse_program();
    ASTPtr parse_top_level();
    ASTPtr parse_main_block();
    ASTPtr parse_veloragame_block();
    ASTPtr parse_game_loop_block();

    // Declarations
    ASTPtr parse_var_decl();
    ASTPtr parse_const_decl();
    ASTPtr parse_func_decl();
    ASTPtr parse_struct_decl();
    ASTPtr parse_class_decl();
    ASTPtr parse_trait_decl();
    ASTPtr parse_param();
    std::vector<ASTPtr> parse_param_list();

    // Statements
    ASTPtr parse_statement();
    ASTPtr parse_if_stmt();
    ASTPtr parse_for_stmt();
    ASTPtr parse_while_stmt();
    ASTPtr parse_loop_stmt();
    ASTPtr parse_match_stmt();
    ASTPtr parse_return_stmt();
    ASTPtr parse_import_stmt();
    ASTPtr parse_from_import_stmt();
    ASTPtr parse_export_stmt();
    ASTPtr parse_with_stmt();
    ASTPtr parse_try_stmt();
    ASTPtr parse_assignment_or_expr();
    ASTPtr parse_decorator();

    // Block
    ASTPtr parse_block();

    // Expressions
    ASTPtr parse_expression();
    ASTPtr parse_or_expr();
    ASTPtr parse_and_expr();
    ASTPtr parse_not_expr();
    ASTPtr parse_comparison();
    ASTPtr parse_bitwise_or();
    ASTPtr parse_bitwise_xor();
    ASTPtr parse_bitwise_and();
    ASTPtr parse_shift();
    ASTPtr parse_addition();
    ASTPtr parse_multiplication();
    ASTPtr parse_unary();
    ASTPtr parse_power();
    ASTPtr parse_postfix();
    ASTPtr parse_primary();
    ASTPtr parse_list_literal();
    ASTPtr parse_dict_literal();
    ASTPtr parse_fstring();

    // Types
    ASTPtr parse_type_annotation();

public:
    Parser(const std::vector<Token>& token_list);
    ASTPtr parse();
};

#endif
