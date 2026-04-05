#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& token_list)
    : tokens(token_list), pos(0) {}

Token Parser::current() {
    if (pos >= (int)tokens.size()) return Token(TokenType::END_OF_FILE, "", 0, 0);
    return tokens[pos];
}

Token Parser::peek(int offset) {
    int target = pos + offset;
    if (target >= (int)tokens.size()) return Token(TokenType::END_OF_FILE, "", 0, 0);
    return tokens[target];
}

Token Parser::advance() {
    Token t = current();
    pos++;
    return t;
}

Token Parser::eat(TokenType expected) {
    Token t = current();
    if (t.type != expected) {
        throw std::runtime_error(
            "line " + std::to_string(t.line) + ": expected token but got '" + t.value + "'"
        );
    }
    return advance();
}

bool Parser::check(TokenType type) {
    return current().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void Parser::skip_newlines() {
    while (check(TokenType::NEWLINE)) advance();
}

void Parser::expect_newline() {
    if (check(TokenType::NEWLINE) || check(TokenType::END_OF_FILE)) {
        skip_newlines();
    }
}

ASTPtr Parser::parse() {
    return parse_program();
}

ASTPtr Parser::parse_program() {
    auto program = make_node(NodeType::PROGRAM, "program", 1, 1);
    skip_newlines();

    while (!check(TokenType::END_OF_FILE)) {
        auto node = parse_top_level();
        if (node) program->add(node);
        skip_newlines();
    }

    return program;
}

ASTPtr Parser::parse_top_level() {
    skip_newlines();

    if (check(TokenType::AT)) {
        return parse_decorator();
    }
    if (check(TokenType::KW_VELORAGAME)) {
        return parse_veloragame_block();
    }
    if (check(TokenType::KW_MAIN)) {
        return parse_main_block();
    }
    if (check(TokenType::KW_FUNC)) {
        return parse_func_decl();
    }
    if (check(TokenType::KW_STRUCT)) {
        return parse_struct_decl();
    }
    if (check(TokenType::KW_CLASS)) {
        return parse_class_decl();
    }
    if (check(TokenType::KW_TRAIT)) {
        return parse_trait_decl();
    }
    if (check(TokenType::KW_IMPORT)) {
        return parse_import_stmt();
    }
    if (check(TokenType::KW_FROM)) {
        return parse_from_import_stmt();
    }
    if (check(TokenType::KW_EXPORT)) {
        return parse_export_stmt();
    }
    if (check(TokenType::KW_MODULE)) {
        auto node = make_node(NodeType::MODULE_STMT, "", current().line, current().column);
        advance();
        node->value = current().value;
        advance();
        while (match(TokenType::DOT)) {
            node->value += "." + current().value;
            advance();
        }
        expect_newline();
        return node;
    }

    return parse_statement();
}

ASTPtr Parser::parse_main_block() {
    auto node = make_node(NodeType::MAIN_BLOCK, "main", current().line, current().column);
    eat(TokenType::KW_MAIN);
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_veloragame_block() {
    auto node = make_node(NodeType::VELORAGAME_BLOCK, "Veloragame", current().line, current().column);
    eat(TokenType::KW_VELORAGAME);

    if (match(TokenType::LPAREN)) {
        while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
            auto param = make_node(NodeType::PARAM, current().value, current().line, current().column);
            advance();
            if (match(TokenType::ASSIGN)) {
                param->add(parse_expression());
            }
            node->add(param);
            match(TokenType::COMMA);
        }
        eat(TokenType::RPAREN);
    }

    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_game_loop_block() {
    auto node = make_node(NodeType::GAME_LOOP_BLOCK, "game_loop", current().line, current().column);
    eat(TokenType::KW_GAME_LOOP);
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_block() {
    auto block = make_node(NodeType::BLOCK, "", current().line, current().column);
    eat(TokenType::INDENT);

    while (!check(TokenType::DEDENT) && !check(TokenType::END_OF_FILE)) {
        skip_newlines();
        if (check(TokenType::DEDENT) || check(TokenType::END_OF_FILE)) break;
        auto stmt = parse_statement();
        if (stmt) block->add(stmt);
        skip_newlines();
    }

    if (check(TokenType::DEDENT)) advance();
    return block;
}

ASTPtr Parser::parse_statement() {
    skip_newlines();

    if (check(TokenType::AT)) return parse_decorator();
    if (check(TokenType::KW_IF)) return parse_if_stmt();
    if (check(TokenType::KW_FOR)) return parse_for_stmt();
    if (check(TokenType::KW_WHILE)) return parse_while_stmt();
    if (check(TokenType::KW_LOOP)) return parse_loop_stmt();
    if (check(TokenType::KW_MATCH)) return parse_match_stmt();
    if (check(TokenType::KW_RETURN)) return parse_return_stmt();
    if (check(TokenType::KW_IMPORT)) return parse_import_stmt();
    if (check(TokenType::KW_FROM)) return parse_from_import_stmt();
    if (check(TokenType::KW_WITH)) return parse_with_stmt();
    if (check(TokenType::KW_TRY)) return parse_try_stmt();
    if (check(TokenType::KW_FUNC)) return parse_func_decl();
    if (check(TokenType::KW_STRUCT)) return parse_struct_decl();
    if (check(TokenType::KW_CLASS)) return parse_class_decl();
    if (check(TokenType::KW_MAIN)) return parse_main_block();
    if (check(TokenType::KW_GAME_LOOP)) return parse_game_loop_block();

    if (check(TokenType::KW_BREAK)) {
        auto node = make_node(NodeType::BREAK_STMT, "break", current().line, current().column);
        advance();
        expect_newline();
        return node;
    }
    if (check(TokenType::KW_CONTINUE)) {
        auto node = make_node(NodeType::CONTINUE_STMT, "continue", current().line, current().column);
        advance();
        expect_newline();
        return node;
    }
    if (check(TokenType::KW_PASS)) {
        auto node = make_node(NodeType::PASS_STMT, "pass", current().line, current().column);
        advance();
        expect_newline();
        return node;
    }

    return parse_assignment_or_expr();
}

ASTPtr Parser::parse_assignment_or_expr() {
    auto expr = parse_expression();

    if (check(TokenType::WALRUS)) {
        auto node = make_node(NodeType::VAR_DECL, "", expr->line, expr->column);
        node->value = expr->value;
        advance();
        node->add(parse_expression());
        expect_newline();
        return node;
    }

    if (check(TokenType::CONST_DECL)) {
        auto node = make_node(NodeType::CONST_DECL, "", expr->line, expr->column);
        node->value = expr->value;
        node->is_mutable = false;
        advance();
        node->add(parse_expression());
        expect_newline();
        return node;
    }

    if (check(TokenType::COLON)) {
        advance();
        auto type_node = parse_type_annotation();
        if (match(TokenType::ASSIGN)) {
            auto node = make_node(NodeType::VAR_DECL, expr->value, expr->line, expr->column);
            node->type_name = type_node->value;
            node->add(type_node);
            node->add(parse_expression());
            expect_newline();
            return node;
        }
        auto node = make_node(NodeType::VAR_DECL, expr->value, expr->line, expr->column);
        node->type_name = type_node->value;
        node->add(type_node);
        expect_newline();
        return node;
    }

    if (check(TokenType::ASSIGN)) {
        auto node = make_node(NodeType::ASSIGN_STMT, "", expr->line, expr->column);
        node->add(expr);
        advance();
        node->add(parse_expression());
        expect_newline();
        return node;
    }

    if (check(TokenType::PLUS_ASSIGN) || check(TokenType::MINUS_ASSIGN) ||
        check(TokenType::STAR_ASSIGN) || check(TokenType::SLASH_ASSIGN)) {
        auto node = make_node(NodeType::COMPOUND_ASSIGN, "", expr->line, expr->column);
        node->op = current().value;
        node->add(expr);
        advance();
        node->add(parse_expression());
        expect_newline();
        return node;
    }

    auto stmt = make_node(NodeType::EXPR_STMT, "", expr->line, expr->column);
    stmt->add(expr);
    expect_newline();
    return stmt;
}

ASTPtr Parser::parse_if_stmt() {
    auto node = make_node(NodeType::IF_STMT, "if", current().line, current().column);
    eat(TokenType::KW_IF);
    node->add(parse_expression());
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());

    while (check(TokenType::KW_ELIF)) {
        auto elif = make_node(NodeType::ELIF_CLAUSE, "elif", current().line, current().column);
        advance();
        elif->add(parse_expression());
        eat(TokenType::COLON);
        expect_newline();
        elif->add(parse_block());
        node->add(elif);
    }

    if (check(TokenType::KW_ELSE)) {
        auto else_node = make_node(NodeType::ELSE_CLAUSE, "else", current().line, current().column);
        advance();
        eat(TokenType::COLON);
        expect_newline();
        else_node->add(parse_block());
        node->add(else_node);
    }

    return node;
}

ASTPtr Parser::parse_for_stmt() {
    auto node = make_node(NodeType::FOR_STMT, "", current().line, current().column);
    eat(TokenType::KW_FOR);

    node->value = current().value;
    advance();

    if (match(TokenType::COMMA)) {
        node->value += "," + current().value;
        advance();
    }

    eat(TokenType::KW_IN);
    node->add(parse_expression());
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_while_stmt() {
    auto node = make_node(NodeType::WHILE_STMT, "while", current().line, current().column);
    eat(TokenType::KW_WHILE);
    node->add(parse_expression());
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_loop_stmt() {
    auto node = make_node(NodeType::LOOP_STMT, "loop", current().line, current().column);
    eat(TokenType::KW_LOOP);
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_match_stmt() {
    auto node = make_node(NodeType::MATCH_STMT, "match", current().line, current().column);
    eat(TokenType::KW_MATCH);
    node->add(parse_expression());
    eat(TokenType::COLON);
    expect_newline();
    eat(TokenType::INDENT);

    while (!check(TokenType::DEDENT) && !check(TokenType::END_OF_FILE)) {
        skip_newlines();
        if (check(TokenType::DEDENT)) break;

        auto arm = make_node(NodeType::MATCH_ARM, "", current().line, current().column);
        arm->add(parse_expression());

        if (check(TokenType::KW_IF)) {
            advance();
            arm->add(parse_expression());
        }

        eat(TokenType::COLON);
        expect_newline();
        arm->add(parse_block());
        node->add(arm);
        skip_newlines();
    }

    if (check(TokenType::DEDENT)) advance();
    return node;
}

ASTPtr Parser::parse_return_stmt() {
    auto node = make_node(NodeType::RETURN_STMT, "return", current().line, current().column);
    eat(TokenType::KW_RETURN);
    if (!check(TokenType::NEWLINE) && !check(TokenType::END_OF_FILE) && !check(TokenType::DEDENT)) {
        node->add(parse_expression());
    }
    expect_newline();
    return node;
}

ASTPtr Parser::parse_import_stmt() {
    auto node = make_node(NodeType::IMPORT_STMT, "", current().line, current().column);
    eat(TokenType::KW_IMPORT);
    node->value = current().value;
    advance();
    while (match(TokenType::DOT)) {
        node->value += "." + current().value;
        advance();
    }
    if (match(TokenType::KW_AS)) {
        node->type_name = current().value;
        advance();
    }
    expect_newline();
    return node;
}

ASTPtr Parser::parse_from_import_stmt() {
    auto node = make_node(NodeType::FROM_IMPORT_STMT, "", current().line, current().column);
    eat(TokenType::KW_FROM);
    node->value = current().value;
    advance();
    while (match(TokenType::DOT)) {
        node->value += "." + current().value;
        advance();
    }
    eat(TokenType::KW_IMPORT);

    auto imported = make_node(NodeType::IDENTIFIER_EXPR, current().value, current().line, current().column);
    advance();
    node->add(imported);

    while (match(TokenType::COMMA)) {
        auto next = make_node(NodeType::IDENTIFIER_EXPR, current().value, current().line, current().column);
        advance();
        node->add(next);
    }

    expect_newline();
    return node;
}

ASTPtr Parser::parse_export_stmt() {
    auto node = make_node(NodeType::EXPORT_STMT, "export", current().line, current().column);
    eat(TokenType::KW_EXPORT);
    node->add(parse_statement());
    return node;
}

ASTPtr Parser::parse_with_stmt() {
    auto node = make_node(NodeType::WITH_STMT, "with", current().line, current().column);
    eat(TokenType::KW_WITH);
    node->add(parse_expression());
    eat(TokenType::KW_AS);
    node->type_name = current().value;
    advance();
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_try_stmt() {
    auto node = make_node(NodeType::TRY_STMT, "try", current().line, current().column);
    eat(TokenType::KW_TRY);
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());

    while (check(TokenType::KW_CATCH)) {
        auto catch_node = make_node(NodeType::CATCH_CLAUSE, "catch", current().line, current().column);
        advance();
        if (!check(TokenType::COLON)) {
            catch_node->value = current().value;
            advance();
        }
        eat(TokenType::COLON);
        expect_newline();
        catch_node->add(parse_block());
        node->add(catch_node);
    }

    return node;
}

ASTPtr Parser::parse_func_decl() {
    auto node = make_node(NodeType::FUNC_DECL, "", current().line, current().column);
    eat(TokenType::KW_FUNC);

    node->value = current().value;
    advance();

    if (match(TokenType::LBRACKET)) {
        while (!check(TokenType::RBRACKET) && !check(TokenType::END_OF_FILE)) {
            auto gen = make_node(NodeType::GENERIC_TYPE, current().value, current().line, current().column);
            advance();
            node->add(gen);
            match(TokenType::COMMA);
        }
        eat(TokenType::RBRACKET);
    }

    eat(TokenType::LPAREN);
    auto params = parse_param_list();
    for (auto& p : params) node->add(p);
    eat(TokenType::RPAREN);

    if (match(TokenType::ARROW)) {
        node->type_name = current().value;
        node->add(parse_type_annotation());
    }

    if (match(TokenType::ASSIGN)) {
        node->add(parse_expression());
        expect_newline();
        return node;
    }

    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

std::vector<ASTPtr> Parser::parse_param_list() {
    std::vector<ASTPtr> params;
    while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
        params.push_back(parse_param());
        if (!check(TokenType::RPAREN)) eat(TokenType::COMMA);
    }
    return params;
}

ASTPtr Parser::parse_param() {
    auto node = make_node(NodeType::PARAM, current().value, current().line, current().column);
    advance();

    if (match(TokenType::COLON)) {
        node->add(parse_type_annotation());
    }

    if (match(TokenType::ASSIGN)) {
        node->add(parse_expression());
    }

    return node;
}

ASTPtr Parser::parse_struct_decl() {
    auto node = make_node(NodeType::STRUCT_DECL, "", current().line, current().column);
    eat(TokenType::KW_STRUCT);
    node->value = current().value;
    advance();
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_class_decl() {
    auto node = make_node(NodeType::CLASS_DECL, "", current().line, current().column);
    eat(TokenType::KW_CLASS);
    node->value = current().value;
    advance();

    if (match(TokenType::LPAREN)) {
        node->type_name = current().value;
        advance();
        eat(TokenType::RPAREN);
    }

    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_trait_decl() {
    auto node = make_node(NodeType::TRAIT_DECL, "", current().line, current().column);
    eat(TokenType::KW_TRAIT);
    node->value = current().value;
    advance();
    eat(TokenType::COLON);
    expect_newline();
    node->add(parse_block());
    return node;
}

ASTPtr Parser::parse_decorator() {
    auto dec = make_node(NodeType::DECORATOR, "", current().line, current().column);
    eat(TokenType::AT);
    dec->value = current().value;
    advance();

    if (match(TokenType::LPAREN)) {
        while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
            dec->add(parse_expression());
            match(TokenType::COMMA);
        }
        eat(TokenType::RPAREN);
    }

    expect_newline();
    skip_newlines();
    dec->add(parse_statement());
    return dec;
}

ASTPtr Parser::parse_type_annotation() {
    auto node = make_node(NodeType::TYPE_ANNOTATION, current().value, current().line, current().column);
    advance();

    if (match(TokenType::LBRACKET)) {
        while (!check(TokenType::RBRACKET) && !check(TokenType::END_OF_FILE)) {
            node->add(parse_type_annotation());
            match(TokenType::COMMA);
        }
        eat(TokenType::RBRACKET);
    }

    return node;
}

// Expression parsing — operator precedence climbing

ASTPtr Parser::parse_expression() {
    return parse_or_expr();
}

ASTPtr Parser::parse_or_expr() {
    auto left = parse_and_expr();
    while (check(TokenType::OR)) {
        auto node = make_node(NodeType::BINARY_EXPR, "or", current().line, current().column);
        node->op = "or";
        advance();
        node->add(left);
        node->add(parse_and_expr());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_and_expr() {
    auto left = parse_not_expr();
    while (check(TokenType::AND)) {
        auto node = make_node(NodeType::BINARY_EXPR, "and", current().line, current().column);
        node->op = "and";
        advance();
        node->add(left);
        node->add(parse_not_expr());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_not_expr() {
    if (check(TokenType::NOT)) {
        auto node = make_node(NodeType::UNARY_EXPR, "not", current().line, current().column);
        node->op = "not";
        advance();
        node->add(parse_not_expr());
        return node;
    }
    return parse_comparison();
}

ASTPtr Parser::parse_comparison() {
    auto left = parse_bitwise_or();
    while (check(TokenType::EQUAL) || check(TokenType::NOT_EQUAL) ||
           check(TokenType::LESS) || check(TokenType::LESS_EQUAL) ||
           check(TokenType::GREATER) || check(TokenType::GREATER_EQUAL) ||
           check(TokenType::KW_IN)) {
        auto node = make_node(NodeType::BINARY_EXPR, current().value, current().line, current().column);
        node->op = current().value;
        advance();
        node->add(left);
        node->add(parse_bitwise_or());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_bitwise_or() {
    auto left = parse_bitwise_xor();
    while (check(TokenType::PIPE)) {
        auto node = make_node(NodeType::BINARY_EXPR, "|", current().line, current().column);
        node->op = "|";
        advance();
        node->add(left);
        node->add(parse_bitwise_xor());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_bitwise_xor() {
    auto left = parse_bitwise_and();
    while (check(TokenType::CARET)) {
        auto node = make_node(NodeType::BINARY_EXPR, "^", current().line, current().column);
        node->op = "^";
        advance();
        node->add(left);
        node->add(parse_bitwise_and());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_bitwise_and() {
    auto left = parse_shift();
    while (check(TokenType::AMPERSAND)) {
        auto node = make_node(NodeType::BINARY_EXPR, "&", current().line, current().column);
        node->op = "&";
        advance();
        node->add(left);
        node->add(parse_shift());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_shift() {
    auto left = parse_addition();
    while (check(TokenType::SHIFT_LEFT) || check(TokenType::SHIFT_RIGHT)) {
        auto node = make_node(NodeType::BINARY_EXPR, current().value, current().line, current().column);
        node->op = current().value;
        advance();
        node->add(left);
        node->add(parse_addition());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_addition() {
    auto left = parse_multiplication();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        auto node = make_node(NodeType::BINARY_EXPR, current().value, current().line, current().column);
        node->op = current().value;
        advance();
        node->add(left);
        node->add(parse_multiplication());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_multiplication() {
    auto left = parse_unary();
    while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT)) {
        auto node = make_node(NodeType::BINARY_EXPR, current().value, current().line, current().column);
        node->op = current().value;
        advance();
        node->add(left);
        node->add(parse_unary());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_unary() {
    if (check(TokenType::MINUS) || check(TokenType::TILDE)) {
        auto node = make_node(NodeType::UNARY_EXPR, current().value, current().line, current().column);
        node->op = current().value;
        advance();
        node->add(parse_unary());
        return node;
    }
    return parse_power();
}

ASTPtr Parser::parse_power() {
    auto left = parse_postfix();
    if (check(TokenType::POWER)) {
        auto node = make_node(NodeType::BINARY_EXPR, "**", current().line, current().column);
        node->op = "**";
        advance();
        node->add(left);
        node->add(parse_unary());
        left = node;
    }
    return left;
}

ASTPtr Parser::parse_postfix() {
    auto left = parse_primary();

    while (true) {
        if (check(TokenType::DOT)) {
            advance();
            auto member = make_node(NodeType::MEMBER_EXPR, current().value, current().line, current().column);
            advance();
            member->add(left);

            if (check(TokenType::LPAREN)) {
                auto call = make_node(NodeType::CALL_EXPR, member->value, member->line, member->column);
                call->add(left);
                advance();
                while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
                    if (check(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
                        auto kwarg = make_node(NodeType::PARAM, current().value, current().line, current().column);
                        advance();
                        advance();
                        kwarg->add(parse_expression());
                        call->add(kwarg);
                    } else {
                        call->add(parse_expression());
                    }
                    match(TokenType::COMMA);
                }
                eat(TokenType::RPAREN);
                left = call;
            } else {
                left = member;
            }
        } else if (check(TokenType::LBRACKET)) {
            advance();
            auto idx = parse_expression();

            if (check(TokenType::DOTDOT)) {
                auto slice = make_node(NodeType::SLICE_EXPR, "", idx->line, idx->column);
                slice->add(left);
                slice->add(idx);
                advance();
                if (!check(TokenType::RBRACKET)) {
                    slice->add(parse_expression());
                }
                eat(TokenType::RBRACKET);
                left = slice;
            } else {
                auto index = make_node(NodeType::INDEX_EXPR, "", idx->line, idx->column);
                index->add(left);
                index->add(idx);
                eat(TokenType::RBRACKET);
                left = index;
            }
        } else if (check(TokenType::LPAREN) && left->type == NodeType::IDENTIFIER_EXPR) {
            auto call = make_node(NodeType::CALL_EXPR, left->value, left->line, left->column);
            advance();
            while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
                if (check(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
                    auto kwarg = make_node(NodeType::PARAM, current().value, current().line, current().column);
                    advance();
                    advance();
                    kwarg->add(parse_expression());
                    call->add(kwarg);
                } else {
                    call->add(parse_expression());
                }
                match(TokenType::COMMA);
            }
            eat(TokenType::RPAREN);
            left = call;
        } else {
            break;
        }
    }

    return left;
}

ASTPtr Parser::parse_primary() {
    Token t = current();

    if (t.type == TokenType::INTEGER) {
        advance();
        return make_node(NodeType::INT_LITERAL, t.value, t.line, t.column);
    }
    if (t.type == TokenType::FLOAT) {
        advance();
        return make_node(NodeType::FLOAT_LITERAL, t.value, t.line, t.column);
    }
    if (t.type == TokenType::STRING) {
        advance();
        return make_node(NodeType::STRING_LITERAL, t.value, t.line, t.column);
    }
    if (t.type == TokenType::FSTRING) {
        advance();
        return make_node(NodeType::FSTRING_EXPR, t.value, t.line, t.column);
    }
    if (t.type == TokenType::BOOL_TRUE) {
        advance();
        return make_node(NodeType::BOOL_LITERAL, "true", t.line, t.column);
    }
    if (t.type == TokenType::BOOL_FALSE) {
        advance();
        return make_node(NodeType::BOOL_LITERAL, "false", t.line, t.column);
    }
    if (t.type == TokenType::COLOR_HEX) {
        advance();
        return make_node(NodeType::COLOR_LITERAL, t.value, t.line, t.column);
    }

    if (t.type == TokenType::KW_CREATE) {
        advance();
        auto create = make_node(NodeType::CREATE_EXPR, current().value, t.line, t.column);
        advance();
        eat(TokenType::LPAREN);
        while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
            if (check(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
                auto kwarg = make_node(NodeType::PARAM, current().value, current().line, current().column);
                advance();
                advance();
                kwarg->add(parse_expression());
                create->add(kwarg);
            } else {
                create->add(parse_expression());
            }
            match(TokenType::COMMA);
        }
        eat(TokenType::RPAREN);
        return create;
    }

    if (t.type == TokenType::LBRACKET) {
        return parse_list_literal();
    }

    if (t.type == TokenType::LBRACE) {
        return parse_dict_literal();
    }

    if (t.type == TokenType::LPAREN) {
        advance();
        auto expr = parse_expression();

        if (check(TokenType::COMMA)) {
            auto tuple_node = make_node(NodeType::TUPLE_LITERAL, "", t.line, t.column);
            tuple_node->add(expr);
            while (match(TokenType::COMMA)) {
                if (check(TokenType::RPAREN)) break;
                tuple_node->add(parse_expression());
            }
            eat(TokenType::RPAREN);
            return tuple_node;
        }

        eat(TokenType::RPAREN);
        return expr;
    }

    if (t.type == TokenType::DOTDOT) {
        auto range = make_node(NodeType::RANGE_EXPR, "..", t.line, t.column);
        advance();
        range->add(parse_expression());
        return range;
    }

    if (t.type == TokenType::IDENTIFIER || (int)t.type >= (int)TokenType::KW_VECTOR2) {
        advance();
        auto id = make_node(NodeType::IDENTIFIER_EXPR, t.value, t.line, t.column);

        if (check(TokenType::DOTDOT)) {
            auto range = make_node(NodeType::RANGE_EXPR, "..", t.line, t.column);
            range->add(id);
            advance();
            if (!check(TokenType::RBRACKET) && !check(TokenType::COLON) &&
                !check(TokenType::NEWLINE) && !check(TokenType::END_OF_FILE)) {
                range->add(parse_expression());
            }
            return range;
        }

        return id;
    }

    if ((int)t.type >= (int)TokenType::KW_INT && (int)t.type <= (int)TokenType::KW_COLOR) {
        advance();
        return make_node(NodeType::IDENTIFIER_EXPR, t.value, t.line, t.column);
    }

    if (t.type == TokenType::KW_DEFAULT) {
        advance();
        return make_node(NodeType::IDENTIFIER_EXPR, "default", t.line, t.column);
    }

    throw std::runtime_error(
        "line " + std::to_string(t.line) + ": unexpected token '" + t.value + "'"
    );
}

ASTPtr Parser::parse_list_literal() {
    auto node = make_node(NodeType::LIST_LITERAL, "", current().line, current().column);
    eat(TokenType::LBRACKET);

    if (check(TokenType::RBRACKET)) {
        eat(TokenType::RBRACKET);
        return node;
    }

    auto first = parse_expression();

    if (check(TokenType::KW_FOR)) {
        auto comp = make_node(NodeType::LIST_COMP, "", node->line, node->column);
        comp->add(first);
        eat(TokenType::KW_FOR);
        comp->value = current().value;
        advance();
        eat(TokenType::KW_IN);
        comp->add(parse_expression());

        if (check(TokenType::KW_IF)) {
            advance();
            comp->add(parse_expression());
        }

        eat(TokenType::RBRACKET);
        return comp;
    }

    node->add(first);
    while (match(TokenType::COMMA)) {
        if (check(TokenType::RBRACKET)) break;
        node->add(parse_expression());
    }
    eat(TokenType::RBRACKET);
    return node;
}

ASTPtr Parser::parse_dict_literal() {
    auto node = make_node(NodeType::DICT_LITERAL, "", current().line, current().column);
    eat(TokenType::LBRACE);

    while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
        auto key = parse_expression();
        eat(TokenType::COLON);
        auto val = parse_expression();
        auto pair = make_node(NodeType::TUPLE_LITERAL, "", key->line, key->column);
        pair->add(key);
        pair->add(val);
        node->add(pair);
        match(TokenType::COMMA);
    }

    eat(TokenType::RBRACE);
    return node;
}
