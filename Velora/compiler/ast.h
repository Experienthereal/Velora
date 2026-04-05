#ifndef VELORA_AST_H
#define VELORA_AST_H

#include <string>
#include <vector>
#include <memory>

enum class NodeType {
    PROGRAM,
    MAIN_BLOCK,
    VELORAGAME_BLOCK,
    GAME_LOOP_BLOCK,

    // Declarations
    VAR_DECL,
    CONST_DECL,
    FUNC_DECL,
    STRUCT_DECL,
    CLASS_DECL,
    TRAIT_DECL,
    PARAM,

    // Statements
    ASSIGN_STMT,
    COMPOUND_ASSIGN,
    IF_STMT,
    ELIF_CLAUSE,
    ELSE_CLAUSE,
    FOR_STMT,
    WHILE_STMT,
    LOOP_STMT,
    MATCH_STMT,
    MATCH_ARM,
    RETURN_STMT,
    BREAK_STMT,
    CONTINUE_STMT,
    PASS_STMT,
    IMPORT_STMT,
    FROM_IMPORT_STMT,
    EXPORT_STMT,
    MODULE_STMT,
    WITH_STMT,
    TRY_STMT,
    CATCH_CLAUSE,
    EXPR_STMT,

    // Expressions
    BINARY_EXPR,
    UNARY_EXPR,
    CALL_EXPR,
    MEMBER_EXPR,
    INDEX_EXPR,
    SLICE_EXPR,
    CREATE_EXPR,
    FSTRING_EXPR,
    LIST_COMP,

    // Literals
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOL_LITERAL,
    COLOR_LITERAL,
    LIST_LITERAL,
    DICT_LITERAL,
    TUPLE_LITERAL,
    IDENTIFIER_EXPR,
    RANGE_EXPR,

    // Types
    TYPE_ANNOTATION,
    GENERIC_TYPE,

    // Decorators
    DECORATOR,

    BLOCK,
};

struct ASTNode {
    NodeType type;
    std::string value;
    int line;
    int column;
    std::vector<std::shared_ptr<ASTNode>> children;

    // Extra fields used by specific node types
    std::string op;
    std::string type_name;
    bool is_mutable = true;

    ASTNode(NodeType t, const std::string& v, int ln, int col)
        : type(t), value(v), line(ln), column(col) {}

    void add(std::shared_ptr<ASTNode> child) {
        if (child) children.push_back(child);
    }
};

using ASTPtr = std::shared_ptr<ASTNode>;

inline ASTPtr make_node(NodeType type, const std::string& value = "",
                        int line = 0, int col = 0) {
    return std::make_shared<ASTNode>(type, value, line, col);
}

#endif
