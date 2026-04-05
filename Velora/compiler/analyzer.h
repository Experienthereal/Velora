#ifndef VELORA_ANALYZER_H
#define VELORA_ANALYZER_H

#include "ast.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>

struct Symbol {
    std::string name;
    std::string type;
    bool is_mutable;
    bool is_function;
    bool is_type;
    int scope_depth;

    Symbol() : is_mutable(true), is_function(false), is_type(false), scope_depth(0) {}
    Symbol(const std::string& n, const std::string& t, bool mut, int depth)
        : name(n), type(t), is_mutable(mut), is_function(false), is_type(false), scope_depth(depth) {}
};

struct AnalyzerError {
    std::string message;
    int line;
    int column;

    AnalyzerError(const std::string& msg, int ln, int col)
        : message(msg), line(ln), column(col) {}
};

class SemanticAnalyzer {
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    std::vector<AnalyzerError> errors;
    std::vector<AnalyzerError> warnings;
    int scope_depth;
    bool in_function;
    bool in_loop;
    bool in_veloragame;
    std::string current_return_type;

    void push_scope();
    void pop_scope();
    void define(const std::string& name, const std::string& type, bool is_mutable);
    void define_function(const std::string& name, const std::string& return_type);
    void define_type(const std::string& name);
    Symbol* lookup(const std::string& name);
    void error(const std::string& msg, int line, int col);
    void warn(const std::string& msg, int line, int col);
    void register_builtins();

    std::string analyze_node(ASTPtr node);
    std::string analyze_program(ASTPtr node);
    std::string analyze_main_block(ASTPtr node);
    std::string analyze_veloragame_block(ASTPtr node);
    std::string analyze_func_decl(ASTPtr node);
    std::string analyze_struct_decl(ASTPtr node);
    std::string analyze_class_decl(ASTPtr node);
    std::string analyze_var_decl(ASTPtr node);
    std::string analyze_const_decl(ASTPtr node);
    std::string analyze_if_stmt(ASTPtr node);
    std::string analyze_for_stmt(ASTPtr node);
    std::string analyze_while_stmt(ASTPtr node);
    std::string analyze_match_stmt(ASTPtr node);
    std::string analyze_return_stmt(ASTPtr node);
    std::string analyze_expression(ASTPtr node);
    std::string analyze_binary(ASTPtr node);
    std::string analyze_call(ASTPtr node);
    std::string analyze_block(ASTPtr node);

public:
    SemanticAnalyzer();
    bool analyze(ASTPtr root);
    const std::vector<AnalyzerError>& get_errors() const { return errors; }
    const std::vector<AnalyzerError>& get_warnings() const { return warnings; }
};

#endif
