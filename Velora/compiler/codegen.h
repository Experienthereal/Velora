#ifndef VELORA_CODEGEN_H
#define VELORA_CODEGEN_H

#include "ast.h"
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>

class CodeGenerator {
    std::stringstream header;
    std::stringstream body;
    std::stringstream functions;
    std::stringstream structs;
    int indent_level;
    bool in_main;
    std::unordered_set<std::string> includes;
    std::unordered_set<std::string> declared_structs;

    std::string indent();
    void emit(std::stringstream& ss, const std::string& code);
    void emit_line(std::stringstream& ss, const std::string& code);
    void add_include(const std::string& inc);

    void gen_node(ASTPtr node);
    void gen_program(ASTPtr node);
    void gen_main_block(ASTPtr node);
    void gen_veloragame_block(ASTPtr node);
    void gen_func_decl(ASTPtr node);
    void gen_struct_decl(ASTPtr node);
    void gen_class_decl(ASTPtr node);
    void gen_var_decl(ASTPtr node);
    void gen_const_decl(ASTPtr node);
    void gen_assign_stmt(ASTPtr node);
    void gen_compound_assign(ASTPtr node);
    void gen_if_stmt(ASTPtr node);
    void gen_for_stmt(ASTPtr node);
    void gen_while_stmt(ASTPtr node);
    void gen_loop_stmt(ASTPtr node);
    void gen_match_stmt(ASTPtr node);
    void gen_return_stmt(ASTPtr node);
    void gen_block(ASTPtr node);
    void gen_expr_stmt(ASTPtr node);

    std::string gen_expr(ASTPtr node);
    std::string gen_binary(ASTPtr node);
    std::string gen_unary(ASTPtr node);
    std::string gen_call(ASTPtr node);
    std::string gen_member(ASTPtr node);
    std::string gen_index(ASTPtr node);
    std::string gen_create(ASTPtr node);
    std::string gen_list_literal(ASTPtr node);
    std::string gen_fstring(ASTPtr node);

    std::string velora_type_to_c(const std::string& vtype);

public:
    CodeGenerator();
    std::string generate(ASTPtr root);
};

#endif
