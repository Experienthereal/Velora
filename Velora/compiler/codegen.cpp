#include "codegen.h"
#include <iostream>

CodeGenerator::CodeGenerator() : indent_level(0), in_main(false) {}

std::string CodeGenerator::indent() {
    return std::string(indent_level * 4, ' ');
}

void CodeGenerator::emit(std::stringstream& ss, const std::string& code) {
    ss << code;
}

void CodeGenerator::emit_line(std::stringstream& ss, const std::string& code) {
    ss << indent() << code << "\n";
}

void CodeGenerator::add_include(const std::string& inc) {
    includes.insert(inc);
}

std::string CodeGenerator::velora_type_to_c(const std::string& vtype) {
    if (vtype == "int" || vtype == "int32") return "int32_t";
    if (vtype == "int8") return "int8_t";
    if (vtype == "int16") return "int16_t";
    if (vtype == "int64") return "int64_t";
    if (vtype == "uint8") return "uint8_t";
    if (vtype == "uint16") return "uint16_t";
    if (vtype == "uint32") return "uint32_t";
    if (vtype == "uint64") return "uint64_t";
    if (vtype == "float") return "float";
    if (vtype == "double") return "double";
    if (vtype == "bool") return "bool";
    if (vtype == "char") return "char";
    if (vtype == "string") return "const char*";
    if (vtype == "void") return "void";
    return "void*";
}

std::string CodeGenerator::generate(ASTPtr root) {
    add_include("<stdio.h>");
    add_include("<stdlib.h>");
    add_include("<stdbool.h>");
    add_include("<stdint.h>");
    add_include("<string.h>");
    add_include("<math.h>");

    gen_node(root);

    std::stringstream output;

    output << "// Velora Compiler Output\n";
    output << "// Auto-generated — do not edit\n\n";

    for (auto& inc : includes) {
        output << "#include " << inc << "\n";
    }
    output << "\n";

    output << "// Math constants\n";
    output << "#define PI 3.14159265358979323846\n";
    output << "#define TAU 6.28318530717958647692\n";
    output << "#define E_CONST 2.71828182845904523536\n";
    output << "#define DEG_TO_RAD 0.01745329251994329577\n";
    output << "#define RAD_TO_DEG 57.2957795130823208768\n\n";

    output << "// Built-in Vector2\n";
    output << "typedef struct { double x; double y; } Vector2;\n";
    output << "typedef struct { double x; double y; double z; } Vector3;\n";
    output << "typedef struct { double x; double y; double z; double w; } Vector4;\n";
    output << "typedef struct { uint8_t r; uint8_t g; uint8_t b; uint8_t a; } Color;\n\n";

    output << "// Vector2 helpers\n";
    output << "Vector2 vec2(double x, double y) { return (Vector2){x, y}; }\n";
    output << "Vector3 vec3(double x, double y, double z) { return (Vector3){x, y, z}; }\n";
    output << "double vec2_length(Vector2 v) { return sqrt(v.x*v.x + v.y*v.y); }\n";
    output << "double vec2_distance(Vector2 a, Vector2 b) {\n";
    output << "    double dx = a.x - b.x; double dy = a.y - b.y;\n";
    output << "    return sqrt(dx*dx + dy*dy);\n";
    output << "}\n";
    output << "Vector2 vec2_lerp(Vector2 a, Vector2 b, double t) {\n";
    output << "    return vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);\n";
    output << "}\n";
    output << "double vec2_dot(Vector2 a, Vector2 b) { return a.x*b.x + a.y*b.y; }\n\n";

    output << "// Color helper\n";
    output << "Color color_from_hex(uint32_t hex) {\n";
    output << "    Color c;\n";
    output << "    c.r = (hex >> 16) & 0xFF;\n";
    output << "    c.g = (hex >> 8) & 0xFF;\n";
    output << "    c.b = hex & 0xFF;\n";
    output << "    c.a = 255;\n";
    output << "    return c;\n";
    output << "}\n\n";

    output << "// Velora print\n";
    output << "void velora_print_str(const char* s) { printf(\"%s\\n\", s); }\n";
    output << "void velora_print_int(int64_t n) { printf(\"%lld\\n\", (long long)n); }\n";
    output << "void velora_print_double(double d) { printf(\"%g\\n\", d); }\n";
    output << "void velora_print_bool(bool b) { printf(\"%s\\n\", b ? \"true\" : \"false\"); }\n\n";

    if (structs.str().size() > 0) {
        output << "// Structs\n";
        output << structs.str() << "\n";
    }

    if (functions.str().size() > 0) {
        output << "// Functions\n";
        output << functions.str() << "\n";
    }

    output << body.str();

    return output.str();
}

void CodeGenerator::gen_node(ASTPtr node) {
    if (!node) return;

    switch (node->type) {
        case NodeType::PROGRAM:      gen_program(node); break;
        case NodeType::MAIN_BLOCK:   gen_main_block(node); break;
        case NodeType::VELORAGAME_BLOCK: gen_veloragame_block(node); break;
        case NodeType::FUNC_DECL:    gen_func_decl(node); break;
        case NodeType::STRUCT_DECL:  gen_struct_decl(node); break;
        case NodeType::CLASS_DECL:   gen_class_decl(node); break;
        case NodeType::VAR_DECL:     gen_var_decl(node); break;
        case NodeType::CONST_DECL:   gen_const_decl(node); break;
        case NodeType::ASSIGN_STMT:  gen_assign_stmt(node); break;
        case NodeType::COMPOUND_ASSIGN: gen_compound_assign(node); break;
        case NodeType::IF_STMT:      gen_if_stmt(node); break;
        case NodeType::FOR_STMT:     gen_for_stmt(node); break;
        case NodeType::WHILE_STMT:   gen_while_stmt(node); break;
        case NodeType::LOOP_STMT:    gen_loop_stmt(node); break;
        case NodeType::MATCH_STMT:   gen_match_stmt(node); break;
        case NodeType::RETURN_STMT:  gen_return_stmt(node); break;
        case NodeType::BLOCK:        gen_block(node); break;
        case NodeType::EXPR_STMT:    gen_expr_stmt(node); break;
        case NodeType::BREAK_STMT:   emit_line(body, "break;"); break;
        case NodeType::CONTINUE_STMT: emit_line(body, "continue;"); break;
        case NodeType::PASS_STMT:    emit_line(body, "// pass"); break;

        case NodeType::DECORATOR:
            if (!node->children.empty()) gen_node(node->children.back());
            break;

        case NodeType::GAME_LOOP_BLOCK:
            emit_line(body, "// game_loop");
            if (!node->children.empty()) gen_node(node->children[0]);
            break;

        case NodeType::WITH_STMT:
            emit_line(body, "{");
            indent_level++;
            if (!node->children.empty()) {
                std::string resource = gen_expr(node->children[0]);
                emit_line(body, "void* " + node->type_name + " = " + resource + ";");
            }
            if (node->children.size() > 1) gen_node(node->children[1]);
            indent_level--;
            emit_line(body, "}");
            break;

        case NodeType::TRY_STMT:
            emit_line(body, "// try-catch (Velora error handling)");
            emit_line(body, "{");
            indent_level++;
            for (auto& child : node->children) gen_node(child);
            indent_level--;
            emit_line(body, "}");
            break;

        case NodeType::CATCH_CLAUSE:
            emit_line(body, "// catch " + node->value);
            for (auto& child : node->children) gen_node(child);
            break;

        default:
            break;
    }
}

void CodeGenerator::gen_program(ASTPtr node) {
    for (auto& child : node->children) {
        gen_node(child);
    }
}

void CodeGenerator::gen_main_block(ASTPtr node) {
    in_main = true;
    emit_line(body, "int main(int argc, char** argv) {");
    indent_level++;

    for (auto& child : node->children) {
        gen_node(child);
    }

    emit_line(body, "return 0;");
    indent_level--;
    emit_line(body, "}");
    in_main = false;
}

void CodeGenerator::gen_veloragame_block(ASTPtr node) {
    emit_line(body, "// Veloragame initialization");
    for (auto& child : node->children) {
        if (child->type == NodeType::PARAM) continue;
        gen_node(child);
    }
}

void CodeGenerator::gen_func_decl(ASTPtr node) {
    std::string ret = node->type_name.empty() ? "void" : velora_type_to_c(node->type_name);
    std::string sig = ret + " " + node->value + "(";

    bool first = true;
    ASTPtr body_node = nullptr;

    for (auto& child : node->children) {
        if (child->type == NodeType::PARAM) {
            if (!first) sig += ", ";
            std::string ptype = "void*";
            if (!child->children.empty() && child->children[0]->type == NodeType::TYPE_ANNOTATION) {
                ptype = velora_type_to_c(child->children[0]->value);
            }
            sig += ptype + " " + child->value;
            first = false;
        } else if (child->type == NodeType::BLOCK) {
            body_node = child;
        } else if (child->type == NodeType::GENERIC_TYPE) {
            continue;
        } else if (child->type == NodeType::TYPE_ANNOTATION) {
            continue;
        } else {
            // One-liner function
            emit_line(functions, sig + ") {");
            indent_level++;
            emit_line(functions, "return " + gen_expr(child) + ";");
            indent_level--;
            emit_line(functions, "}");
            emit(functions, "\n");
            return;
        }
    }

    sig += ")";
    emit_line(functions, sig + " {");
    indent_level++;

    if (body_node) {
        auto saved = body.str();
        body.str("");
        gen_block(body_node);
        functions << body.str();
        body.str(saved);
    }

    indent_level--;
    emit_line(functions, "}");
    emit(functions, "\n");
}

void CodeGenerator::gen_struct_decl(ASTPtr node) {
    if (declared_structs.count(node->value)) return;
    declared_structs.insert(node->value);

    emit_line(structs, "typedef struct {");
    indent_level++;

    for (auto& child : node->children) {
        if (child->type == NodeType::BLOCK) {
            for (auto& field : child->children) {
                if (field->type == NodeType::VAR_DECL) {
                    std::string ftype = field->type_name.empty() ? "void*" : velora_type_to_c(field->type_name);
                    emit_line(structs, ftype + " " + field->value + ";");
                } else if (field->type == NodeType::FUNC_DECL) {
                    // Methods stored separately
                }
            }
        }
    }

    indent_level--;
    emit_line(structs, "} " + node->value + ";");
    emit(structs, "\n");
}

void CodeGenerator::gen_class_decl(ASTPtr node) {
    gen_struct_decl(node);
}

void CodeGenerator::gen_var_decl(ASTPtr node) {
    std::string type_str = "auto";
    std::string init_expr;

    for (auto& child : node->children) {
        if (child->type == NodeType::TYPE_ANNOTATION) {
            type_str = velora_type_to_c(child->value);
        } else {
            init_expr = gen_expr(child);
        }
    }

    if (type_str == "auto") {
        type_str = "auto";
    }

    std::string line_str;
    if (!init_expr.empty()) {
        line_str = type_str + " " + node->value + " = " + init_expr + ";";
    } else {
        line_str = type_str + " " + node->value + ";";
    }

    emit_line(body, line_str);
}

void CodeGenerator::gen_const_decl(ASTPtr node) {
    std::string init_expr;
    for (auto& child : node->children) {
        init_expr = gen_expr(child);
    }
    emit_line(body, "const auto " + node->value + " = " + init_expr + ";");
}

void CodeGenerator::gen_assign_stmt(ASTPtr node) {
    if (node->children.size() >= 2) {
        std::string target = gen_expr(node->children[0]);
        std::string value = gen_expr(node->children[1]);
        emit_line(body, target + " = " + value + ";");
    }
}

void CodeGenerator::gen_compound_assign(ASTPtr node) {
    if (node->children.size() >= 2) {
        std::string target = gen_expr(node->children[0]);
        std::string value = gen_expr(node->children[1]);
        emit_line(body, target + " " + node->op + " " + value + ";");
    }
}

void CodeGenerator::gen_if_stmt(ASTPtr node) {
    bool first = true;
    for (size_t i = 0; i < node->children.size(); i++) {
        auto child = node->children[i];

        if (i == 0) {
            emit_line(body, "if (" + gen_expr(child) + ") {");
        } else if (child->type == NodeType::ELIF_CLAUSE) {
            if (!child->children.empty()) {
                emit_line(body, "} else if (" + gen_expr(child->children[0]) + ") {");
                indent_level++;
                if (child->children.size() > 1) gen_node(child->children[1]);
                indent_level--;
            }
            continue;
        } else if (child->type == NodeType::ELSE_CLAUSE) {
            emit_line(body, "} else {");
            indent_level++;
            for (auto& c : child->children) gen_node(c);
            indent_level--;
            emit_line(body, "}");
            continue;
        } else if (child->type == NodeType::BLOCK) {
            indent_level++;
            gen_block(child);
            indent_level--;

            if (i + 1 < node->children.size()) {
                continue;
            }
            emit_line(body, "}");
            continue;
        }
    }

    bool has_else = false;
    for (auto& c : node->children) {
        if (c->type == NodeType::ELSE_CLAUSE || c->type == NodeType::ELIF_CLAUSE) {
            has_else = true;
            break;
        }
    }
    if (!has_else && node->children.size() >= 2) {
        emit_line(body, "}");
    }
}

void CodeGenerator::gen_for_stmt(ASTPtr node) {
    if (!node->children.empty() && node->children[0]->type == NodeType::RANGE_EXPR) {
        auto range = node->children[0];
        std::string start = "0";
        std::string end = "0";
        if (range->children.size() == 1) {
            end = gen_expr(range->children[0]);
        } else if (range->children.size() >= 2) {
            start = gen_expr(range->children[0]);
            end = gen_expr(range->children[1]);
        }
        emit_line(body, "for (int " + node->value + " = " + start + "; " +
                  node->value + " < " + end + "; " + node->value + "++) {");
    } else {
        emit_line(body, "// for " + node->value + " in collection");
        emit_line(body, "{");
    }

    indent_level++;
    if (node->children.size() > 1) {
        gen_node(node->children[1]);
    }
    indent_level--;
    emit_line(body, "}");
}

void CodeGenerator::gen_while_stmt(ASTPtr node) {
    std::string cond = node->children.empty() ? "1" : gen_expr(node->children[0]);
    emit_line(body, "while (" + cond + ") {");
    indent_level++;
    if (node->children.size() > 1) gen_node(node->children[1]);
    indent_level--;
    emit_line(body, "}");
}

void CodeGenerator::gen_loop_stmt(ASTPtr node) {
    emit_line(body, "while (1) {");
    indent_level++;
    for (auto& child : node->children) gen_node(child);
    indent_level--;
    emit_line(body, "}");
}

void CodeGenerator::gen_match_stmt(ASTPtr node) {
    if (node->children.empty()) return;
    std::string subject = gen_expr(node->children[0]);

    bool first = true;
    for (size_t i = 1; i < node->children.size(); i++) {
        auto arm = node->children[i];
        if (arm->children.empty()) continue;

        std::string pattern = gen_expr(arm->children[0]);

        if (pattern == "default") {
            emit_line(body, (first ? "" : "} ") + std::string("else {"));
        } else {
            std::string kw = first ? "if" : "} else if";
            emit_line(body, kw + " (" + subject + " == " + pattern + ") {");
        }

        indent_level++;
        for (size_t j = 1; j < arm->children.size(); j++) {
            gen_node(arm->children[j]);
        }
        indent_level--;
        first = false;
    }
    emit_line(body, "}");
}

void CodeGenerator::gen_return_stmt(ASTPtr node) {
    if (node->children.empty()) {
        emit_line(body, "return;");
    } else {
        emit_line(body, "return " + gen_expr(node->children[0]) + ";");
    }
}

void CodeGenerator::gen_block(ASTPtr node) {
    for (auto& child : node->children) {
        gen_node(child);
    }
}

void CodeGenerator::gen_expr_stmt(ASTPtr node) {
    if (!node->children.empty()) {
        emit_line(body, gen_expr(node->children[0]) + ";");
    }
}

std::string CodeGenerator::gen_expr(ASTPtr node) {
    if (!node) return "";

    switch (node->type) {
        case NodeType::INT_LITERAL:
            return node->value;
        case NodeType::FLOAT_LITERAL:
            return node->value;
        case NodeType::STRING_LITERAL:
            return "\"" + node->value + "\"";
        case NodeType::BOOL_LITERAL:
            return node->value;
        case NodeType::COLOR_LITERAL: {
            std::string hex = node->value.substr(1);
            return "color_from_hex(0x" + hex + ")";
        }
        case NodeType::IDENTIFIER_EXPR:
            return node->value;
        case NodeType::FSTRING_EXPR:
            return gen_fstring(node);
        case NodeType::BINARY_EXPR:
            return gen_binary(node);
        case NodeType::UNARY_EXPR:
            return gen_unary(node);
        case NodeType::CALL_EXPR:
            return gen_call(node);
        case NodeType::MEMBER_EXPR:
            return gen_member(node);
        case NodeType::INDEX_EXPR:
            return gen_index(node);
        case NodeType::CREATE_EXPR:
            return gen_create(node);
        case NodeType::LIST_LITERAL:
            return gen_list_literal(node);
        case NodeType::RANGE_EXPR: {
            if (node->children.size() >= 2) {
                return gen_expr(node->children[0]) + ".." + gen_expr(node->children[1]);
            } else if (node->children.size() == 1) {
                return "0.." + gen_expr(node->children[0]);
            }
            return "0..0";
        }
        case NodeType::TUPLE_LITERAL: {
            std::string r = "(";
            for (size_t i = 0; i < node->children.size(); i++) {
                if (i > 0) r += ", ";
                r += gen_expr(node->children[i]);
            }
            return r + ")";
        }
        case NodeType::LIST_COMP:
            return "/* list comprehension */NULL";
        case NodeType::SLICE_EXPR:
            return "/* slice */NULL";
        case NodeType::PARAM:
            if (!node->children.empty()) return gen_expr(node->children[0]);
            return node->value;
        case NodeType::TYPE_ANNOTATION:
            return node->value;
        default:
            return "/* unknown expr */0";
    }
}

std::string CodeGenerator::gen_binary(ASTPtr node) {
    if (node->children.size() < 2) return "0";
    std::string left = gen_expr(node->children[0]);
    std::string right = gen_expr(node->children[1]);
    std::string op = node->op;

    if (op == "and") op = "&&";
    if (op == "or") op = "||";
    if (op == "**") return "pow(" + left + ", " + right + ")";

    return "(" + left + " " + op + " " + right + ")";
}

std::string CodeGenerator::gen_unary(ASTPtr node) {
    if (node->children.empty()) return "0";
    std::string operand = gen_expr(node->children[0]);
    if (node->op == "not") return "(!" + operand + ")";
    return "(" + node->op + operand + ")";
}

std::string CodeGenerator::gen_call(ASTPtr node) {
    std::string name = node->value;

    if (name == "print" || name == "println") {
        if (!node->children.empty()) {
            auto arg = node->children[0];
            if (arg->type == NodeType::STRING_LITERAL || arg->type == NodeType::FSTRING_EXPR) {
                return "velora_print_str(" + gen_expr(arg) + ")";
            }
            if (arg->type == NodeType::INT_LITERAL) {
                return "velora_print_int(" + gen_expr(arg) + ")";
            }
            if (arg->type == NodeType::FLOAT_LITERAL) {
                return "velora_print_double(" + gen_expr(arg) + ")";
            }
            if (arg->type == NodeType::BOOL_LITERAL) {
                return "velora_print_bool(" + gen_expr(arg) + ")";
            }
            return "velora_print_str(" + gen_expr(arg) + ")";
        }
        return "velora_print_str(\"\")";
    }

    if (name == "Vector2") {
        std::string args;
        for (size_t i = 0; i < node->children.size(); i++) {
            if (i > 0) args += ", ";
            args += gen_expr(node->children[i]);
        }
        return "vec2(" + args + ")";
    }
    if (name == "Vector3") {
        std::string args;
        for (size_t i = 0; i < node->children.size(); i++) {
            if (i > 0) args += ", ";
            args += gen_expr(node->children[i]);
        }
        return "vec3(" + args + ")";
    }

    std::string args;
    for (size_t i = 0; i < node->children.size(); i++) {
        auto child = node->children[i];
        if (child->type == NodeType::IDENTIFIER_EXPR && child->value == node->value) continue;
        if (i > 0 && !args.empty()) args += ", ";
        if (child->type == NodeType::PARAM && !child->children.empty()) {
            args += gen_expr(child->children[0]);
        } else {
            args += gen_expr(child);
        }
    }

    return name + "(" + args + ")";
}

std::string CodeGenerator::gen_member(ASTPtr node) {
    if (!node->children.empty()) {
        return gen_expr(node->children[0]) + "." + node->value;
    }
    return node->value;
}

std::string CodeGenerator::gen_index(ASTPtr node) {
    if (node->children.size() >= 2) {
        return gen_expr(node->children[0]) + "[" + gen_expr(node->children[1]) + "]";
    }
    return "NULL";
}

std::string CodeGenerator::gen_create(ASTPtr node) {
    std::string args;
    for (size_t i = 0; i < node->children.size(); i++) {
        if (i > 0) args += ", ";
        auto child = node->children[i];
        if (child->type == NodeType::PARAM && !child->children.empty()) {
            args += gen_expr(child->children[0]);
        } else {
            args += gen_expr(child);
        }
    }
    return node->value + "_create(" + args + ")";
}

std::string CodeGenerator::gen_list_literal(ASTPtr node) {
    std::string items = "{";
    for (size_t i = 0; i < node->children.size(); i++) {
        if (i > 0) items += ", ";
        items += gen_expr(node->children[i]);
    }
    return items + "}";
}

std::string CodeGenerator::gen_fstring(ASTPtr node) {
    add_include("<stdio.h>");
    return "\"" + node->value + "\"";
}
