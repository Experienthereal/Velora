#include "analyzer.h"
#include <iostream>

SemanticAnalyzer::SemanticAnalyzer()
    : scope_depth(0), in_function(false), in_loop(false), in_veloragame(false) {
    push_scope();
    register_builtins();
}

void SemanticAnalyzer::push_scope() {
    scopes.push_back({});
    scope_depth++;
}

void SemanticAnalyzer::pop_scope() {
    scopes.pop_back();
    scope_depth--;
}

void SemanticAnalyzer::define(const std::string& name, const std::string& type, bool is_mutable) {
    Symbol sym(name, type, is_mutable, scope_depth);
    scopes.back()[name] = sym;
}

void SemanticAnalyzer::define_function(const std::string& name, const std::string& return_type) {
    Symbol sym(name, return_type, false, scope_depth);
    sym.is_function = true;
    scopes.back()[name] = sym;
}

void SemanticAnalyzer::define_type(const std::string& name) {
    Symbol sym(name, "type", false, scope_depth);
    sym.is_type = true;
    scopes.back()[name] = sym;
}

Symbol* SemanticAnalyzer::lookup(const std::string& name) {
    for (int i = (int)scopes.size() - 1; i >= 0; i--) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return &it->second;
    }
    return nullptr;
}

void SemanticAnalyzer::error(const std::string& msg, int line, int col) {
    errors.push_back(AnalyzerError(msg, line, col));
}

void SemanticAnalyzer::warn(const std::string& msg, int line, int col) {
    warnings.push_back(AnalyzerError(msg, line, col));
}

void SemanticAnalyzer::register_builtins() {
    define_function("print", "void");
    define_function("println", "void");
    define_function("input", "string");
    define_function("len", "int");
    define_function("str", "string");
    define_function("int", "int");
    define_function("float", "float");
    define_function("double", "double");
    define_function("bool", "bool");
    define_function("abs", "double");
    define_function("sqrt", "double");
    define_function("sin", "double");
    define_function("cos", "double");
    define_function("tan", "double");
    define_function("pow", "double");
    define_function("min", "double");
    define_function("max", "double");
    define_function("clamp", "double");
    define_function("round", "int");
    define_function("floor", "int");
    define_function("ceil", "int");
    define_function("random", "double");
    define_function("randint", "int");
    define_function("open", "File");
    define_function("enumerate", "list");
    define_function("range", "list");
    define_function("playSound", "void");
    define_function("playMusic", "void");
    define_function("stopMusic", "void");

    define_type("int");
    define_type("int8");
    define_type("int16");
    define_type("int32");
    define_type("int64");
    define_type("uint8");
    define_type("uint16");
    define_type("uint32");
    define_type("uint64");
    define_type("float");
    define_type("double");
    define_type("bool");
    define_type("char");
    define_type("string");
    define_type("rawstring");
    define_type("list");
    define_type("array");
    define_type("dict");
    define_type("set");
    define_type("tuple");
    define_type("optional");
    define_type("Vector2");
    define_type("Vector3");
    define_type("Vector4");
    define_type("Matrix3");
    define_type("Matrix4");
    define_type("Quaternion");
    define_type("Color");
    define_type("Window");
    define_type("Renderer");
    define_type("Sprite");
    define_type("Texture");
    define_type("Mesh");
    define_type("Camera");
    define_type("Sound");
    define_type("Music");
    define_type("Font");
    define_type("Text");
    define_type("Button");
    define_type("Panel");
    define_type("Label");
    define_type("Slider");
    define_type("Collider");
    define_type("File");
    define_type("Player");
    define_type("Enemy");

    define("keyboard", "Input", false);
    define("mouse", "Input", false);
    define("controller", "Input", false);
    define("PI", "double", false);
    define("TAU", "double", false);
    define("E", "double", false);
    define("DEG_TO_RAD", "double", false);
    define("RAD_TO_DEG", "double", false);
    define("Vulkan", "GraphicsBackend", false);
    define("DirectX", "GraphicsBackend", false);
    define("OpenGL", "GraphicsBackend", false);
    define("Auto", "GraphicsBackend", false);
}

bool SemanticAnalyzer::analyze(ASTPtr root) {
    analyze_node(root);
    return errors.empty();
}

std::string SemanticAnalyzer::analyze_node(ASTPtr node) {
    if (!node) return "void";

    switch (node->type) {
        case NodeType::PROGRAM:        return analyze_program(node);
        case NodeType::MAIN_BLOCK:     return analyze_main_block(node);
        case NodeType::VELORAGAME_BLOCK: return analyze_veloragame_block(node);
        case NodeType::FUNC_DECL:      return analyze_func_decl(node);
        case NodeType::STRUCT_DECL:    return analyze_struct_decl(node);
        case NodeType::CLASS_DECL:     return analyze_class_decl(node);
        case NodeType::VAR_DECL:       return analyze_var_decl(node);
        case NodeType::CONST_DECL:     return analyze_const_decl(node);
        case NodeType::IF_STMT:        return analyze_if_stmt(node);
        case NodeType::FOR_STMT:       return analyze_for_stmt(node);
        case NodeType::WHILE_STMT:     return analyze_while_stmt(node);
        case NodeType::MATCH_STMT:     return analyze_match_stmt(node);
        case NodeType::RETURN_STMT:    return analyze_return_stmt(node);
        case NodeType::BLOCK:          return analyze_block(node);

        case NodeType::BREAK_STMT:
        case NodeType::CONTINUE_STMT:
            if (!in_loop) error("break/continue outside of loop", node->line, node->column);
            return "void";

        case NodeType::PASS_STMT:
            return "void";

        case NodeType::IMPORT_STMT:
        case NodeType::FROM_IMPORT_STMT:
        case NodeType::EXPORT_STMT:
        case NodeType::MODULE_STMT:
            return "void";

        case NodeType::ASSIGN_STMT: {
            if (node->children.size() >= 2) {
                auto target = node->children[0];
                if (target->type == NodeType::IDENTIFIER_EXPR) {
                    auto sym = lookup(target->value);
                    if (sym && !sym->is_mutable) {
                        error("cannot assign to constant '" + target->value + "'", node->line, node->column);
                    }
                }
                analyze_node(node->children[1]);
            }
            return "void";
        }

        case NodeType::COMPOUND_ASSIGN: {
            if (node->children.size() >= 2) {
                auto target = node->children[0];
                if (target->type == NodeType::IDENTIFIER_EXPR) {
                    auto sym = lookup(target->value);
                    if (sym && !sym->is_mutable) {
                        error("cannot modify constant '" + target->value + "'", node->line, node->column);
                    }
                }
                analyze_node(node->children[1]);
            }
            return "void";
        }

        case NodeType::EXPR_STMT:
            if (!node->children.empty()) return analyze_node(node->children[0]);
            return "void";

        case NodeType::WITH_STMT:
            push_scope();
            if (!node->children.empty()) analyze_node(node->children[0]);
            define(node->type_name, "auto", true);
            if (node->children.size() > 1) analyze_node(node->children[1]);
            pop_scope();
            return "void";

        case NodeType::TRY_STMT:
            for (auto& child : node->children) analyze_node(child);
            return "void";

        case NodeType::CATCH_CLAUSE:
            push_scope();
            if (!node->value.empty()) define(node->value, "Error", true);
            for (auto& child : node->children) analyze_node(child);
            pop_scope();
            return "void";

        case NodeType::GAME_LOOP_BLOCK: {
            bool was_in_loop = in_loop;
            in_loop = true;
            for (auto& child : node->children) analyze_node(child);
            in_loop = was_in_loop;
            return "void";
        }

        case NodeType::DECORATOR:
            if (node->children.size() > 0) analyze_node(node->children.back());
            return "void";

        case NodeType::TRAIT_DECL:
            push_scope();
            define_type(node->value);
            for (auto& child : node->children) analyze_node(child);
            pop_scope();
            return "void";

        default:
            return analyze_expression(node);
    }
}

std::string SemanticAnalyzer::analyze_program(ASTPtr node) {
    for (auto& child : node->children) {
        analyze_node(child);
    }
    return "void";
}

std::string SemanticAnalyzer::analyze_main_block(ASTPtr node) {
    push_scope();
    for (auto& child : node->children) analyze_node(child);
    pop_scope();
    return "void";
}

std::string SemanticAnalyzer::analyze_veloragame_block(ASTPtr node) {
    bool was = in_veloragame;
    in_veloragame = true;
    push_scope();
    for (auto& child : node->children) analyze_node(child);
    pop_scope();
    in_veloragame = was;
    return "void";
}

std::string SemanticAnalyzer::analyze_func_decl(ASTPtr node) {
    std::string ret_type = node->type_name.empty() ? "void" : node->type_name;
    define_function(node->value, ret_type);

    push_scope();
    bool was_fn = in_function;
    std::string old_ret = current_return_type;
    in_function = true;
    current_return_type = ret_type;

    for (auto& child : node->children) {
        if (child->type == NodeType::PARAM) {
            std::string ptype = "auto";
            if (!child->children.empty() && child->children[0]->type == NodeType::TYPE_ANNOTATION) {
                ptype = child->children[0]->value;
            }
            define(child->value, ptype, true);
        } else if (child->type == NodeType::GENERIC_TYPE) {
            define_type(child->value);
        } else {
            analyze_node(child);
        }
    }

    in_function = was_fn;
    current_return_type = old_ret;
    pop_scope();
    return "void";
}

std::string SemanticAnalyzer::analyze_struct_decl(ASTPtr node) {
    define_type(node->value);
    push_scope();
    for (auto& child : node->children) analyze_node(child);
    pop_scope();
    return "void";
}

std::string SemanticAnalyzer::analyze_class_decl(ASTPtr node) {
    if (!node->type_name.empty()) {
        auto parent = lookup(node->type_name);
        if (!parent || !parent->is_type) {
            warn("parent class '" + node->type_name + "' not found", node->line, node->column);
        }
    }
    define_type(node->value);
    push_scope();
    for (auto& child : node->children) analyze_node(child);
    pop_scope();
    return "void";
}

std::string SemanticAnalyzer::analyze_var_decl(ASTPtr node) {
    std::string inferred_type = "auto";

    if (!node->type_name.empty()) {
        inferred_type = node->type_name;
    }

    for (auto& child : node->children) {
        if (child->type != NodeType::TYPE_ANNOTATION) {
            std::string expr_type = analyze_node(child);
            if (inferred_type == "auto") inferred_type = expr_type;
        }
    }

    define(node->value, inferred_type, node->is_mutable);
    return "void";
}

std::string SemanticAnalyzer::analyze_const_decl(ASTPtr node) {
    std::string inferred = "auto";
    for (auto& child : node->children) {
        inferred = analyze_node(child);
    }
    define(node->value, inferred, false);
    return "void";
}

std::string SemanticAnalyzer::analyze_if_stmt(ASTPtr node) {
    for (auto& child : node->children) analyze_node(child);
    return "void";
}

std::string SemanticAnalyzer::analyze_for_stmt(ASTPtr node) {
    push_scope();
    std::string var_name = node->value;
    if (var_name.find(',') != std::string::npos) {
        auto comma_pos = var_name.find(',');
        define(var_name.substr(0, comma_pos), "int", true);
        define(var_name.substr(comma_pos + 1), "auto", true);
    } else {
        define(var_name, "auto", true);
    }

    bool was_loop = in_loop;
    in_loop = true;
    for (auto& child : node->children) analyze_node(child);
    in_loop = was_loop;
    pop_scope();
    return "void";
}

std::string SemanticAnalyzer::analyze_while_stmt(ASTPtr node) {
    bool was_loop = in_loop;
    in_loop = true;
    for (auto& child : node->children) analyze_node(child);
    in_loop = was_loop;
    return "void";
}

std::string SemanticAnalyzer::analyze_match_stmt(ASTPtr node) {
    for (auto& child : node->children) analyze_node(child);
    return "void";
}

std::string SemanticAnalyzer::analyze_return_stmt(ASTPtr node) {
    if (!in_function) {
        error("return outside of function", node->line, node->column);
    }
    if (!node->children.empty()) {
        analyze_node(node->children[0]);
    }
    return "void";
}

std::string SemanticAnalyzer::analyze_block(ASTPtr node) {
    push_scope();
    for (auto& child : node->children) analyze_node(child);
    pop_scope();
    return "void";
}

std::string SemanticAnalyzer::analyze_expression(ASTPtr node) {
    if (!node) return "void";

    switch (node->type) {
        case NodeType::INT_LITERAL:     return "int";
        case NodeType::FLOAT_LITERAL:   return "double";
        case NodeType::STRING_LITERAL:  return "string";
        case NodeType::FSTRING_EXPR:    return "string";
        case NodeType::BOOL_LITERAL:    return "bool";
        case NodeType::COLOR_LITERAL:   return "Color";
        case NodeType::LIST_LITERAL:    return "list";
        case NodeType::DICT_LITERAL:    return "dict";
        case NodeType::TUPLE_LITERAL:   return "tuple";
        case NodeType::RANGE_EXPR:      return "range";
        case NodeType::LIST_COMP:       return "list";

        case NodeType::IDENTIFIER_EXPR: {
            auto sym = lookup(node->value);
            if (!sym) {
                if (node->value != "self") {
                    warn("undeclared identifier '" + node->value + "'", node->line, node->column);
                }
                return "auto";
            }
            return sym->type;
        }

        case NodeType::BINARY_EXPR:
            return analyze_binary(node);

        case NodeType::UNARY_EXPR:
            if (!node->children.empty()) return analyze_node(node->children[0]);
            return "auto";

        case NodeType::CALL_EXPR:
            return analyze_call(node);

        case NodeType::MEMBER_EXPR:
            if (!node->children.empty()) analyze_node(node->children[0]);
            return "auto";

        case NodeType::INDEX_EXPR:
            for (auto& c : node->children) analyze_node(c);
            return "auto";

        case NodeType::SLICE_EXPR:
            for (auto& c : node->children) analyze_node(c);
            return "list";

        case NodeType::CREATE_EXPR: {
            auto sym = lookup(node->value);
            if (!sym || !sym->is_type) {
                warn("unknown type '" + node->value + "' in create expression", node->line, node->column);
            }
            for (auto& c : node->children) analyze_node(c);
            return node->value;
        }

        default:
            for (auto& c : node->children) analyze_node(c);
            return "auto";
    }
}

std::string SemanticAnalyzer::analyze_binary(ASTPtr node) {
    std::string left_type = "auto", right_type = "auto";
    if (node->children.size() >= 1) left_type = analyze_node(node->children[0]);
    if (node->children.size() >= 2) right_type = analyze_node(node->children[1]);

    if (node->op == "==" || node->op == "!=" || node->op == "<" ||
        node->op == "<=" || node->op == ">" || node->op == ">=" ||
        node->op == "and" || node->op == "or" || node->op == "in") {
        return "bool";
    }

    if (node->op == "+" && (left_type == "string" || right_type == "string")) {
        return "string";
    }

    if (left_type == "double" || right_type == "double") return "double";
    if (left_type == "float" || right_type == "float") return "float";
    return left_type;
}

std::string SemanticAnalyzer::analyze_call(ASTPtr node) {
    auto sym = lookup(node->value);
    for (auto& child : node->children) {
        if (child->type == NodeType::PARAM) {
            for (auto& c : child->children) analyze_node(c);
        } else {
            analyze_node(child);
        }
    }

    if (sym && sym->is_function) return sym->type;
    if (sym && sym->is_type) return node->value;
    return "auto";
}
