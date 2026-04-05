#include "lexer.h"
#include "parser.h"
#include "analyzer.h"
#include "codegen.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "error: cannot open file '" << path << "'" << std::endl;
        exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "error: cannot write to '" << path << "'" << std::endl;
        exit(1);
    }
    file << content;
}

void print_version() {
    std::cout << "Velora 0.1.0" << std::endl;
    std::cout << "The Velora Programming Language Compiler" << std::endl;
    std::cout << "https://velora.dev" << std::endl;
}

void print_help() {
    std::cout << "Velora Programming Language" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  velora run <file.vel>         Build and run a Velora program" << std::endl;
    std::cout << "  velora build <file.vel>       Compile to native executable" << std::endl;
    std::cout << "  velora check <file.vel>       Check for errors without compiling" << std::endl;
    std::cout << "  velora tokens <file.vel>      Show tokenized output" << std::endl;
    std::cout << "  velora new <project>          Create a new Velora project" << std::endl;
    std::cout << "  velora version                Show version" << std::endl;
    std::cout << "  velora help                   Show this help" << std::endl;
}

void print_tokens(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    for (auto& t : tokens) {
        std::string type_name;
        switch (t.type) {
            case TokenType::INTEGER:    type_name = "INTEGER"; break;
            case TokenType::FLOAT:      type_name = "FLOAT"; break;
            case TokenType::STRING:     type_name = "STRING"; break;
            case TokenType::FSTRING:    type_name = "FSTRING"; break;
            case TokenType::IDENTIFIER: type_name = "IDENTIFIER"; break;
            case TokenType::WALRUS:     type_name = "WALRUS(:=)"; break;
            case TokenType::CONST_DECL: type_name = "CONST(::)"; break;
            case TokenType::COLON:      type_name = "COLON"; break;
            case TokenType::ARROW:      type_name = "ARROW(->)"; break;
            case TokenType::INDENT:     type_name = "INDENT"; break;
            case TokenType::DEDENT:     type_name = "DEDENT"; break;
            case TokenType::NEWLINE:    type_name = "NEWLINE"; break;
            case TokenType::END_OF_FILE: type_name = "EOF"; break;
            case TokenType::COLOR_HEX:  type_name = "COLOR"; break;
            case TokenType::ERROR:      type_name = "ERROR"; break;
            default:
                if ((int)t.type >= (int)TokenType::KW_FUNC && (int)t.type <= (int)TokenType::KW_REF)
                    type_name = "KEYWORD";
                else if ((int)t.type >= (int)TokenType::KW_INT && (int)t.type <= (int)TokenType::KW_COLOR)
                    type_name = "TYPE";
                else
                    type_name = "OP";
                break;
        }
        if (t.type != TokenType::NEWLINE && t.type != TokenType::END_OF_FILE) {
            std::cout << "  [" << t.line << ":" << t.column << "] "
                      << type_name << " = '" << t.value << "'" << std::endl;
        }
    }
}

int compile_and_run(const std::string& source_path, bool run_after) {
    std::string source = read_file(source_path);

    std::cout << "[vlc] Lexing..." << std::endl;
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    std::cout << "[vlc] Parsing..." << std::endl;
    Parser parser(tokens);
    auto ast = parser.parse();

    std::cout << "[vlc] Analyzing..." << std::endl;
    SemanticAnalyzer analyzer;
    bool ok = analyzer.analyze(ast);

    for (auto& w : analyzer.get_warnings()) {
        std::cerr << "  warning [" << w.line << "]: " << w.message << std::endl;
    }
    for (auto& e : analyzer.get_errors()) {
        std::cerr << "  error [" << e.line << "]: " << e.message << std::endl;
    }

    if (!ok) {
        std::cerr << "[vlc] Compilation failed with " << analyzer.get_errors().size()
                  << " error(s)." << std::endl;
        return 1;
    }

    std::cout << "[vlc] Generating code..." << std::endl;
    CodeGenerator codegen;
    std::string c_code = codegen.generate(ast);

    fs::path src(source_path);
    std::string base = src.stem().string();
    std::string c_file = base + ".c";
    std::string exe_file = base;

    #ifdef _WIN32
        exe_file += ".exe";
    #endif

    write_file(c_file, c_code);
    std::cout << "[vlc] Compiling native binary..." << std::endl;

    std::string compile_cmd = "gcc -O2 -o " + exe_file + " " + c_file + " -lm";

    #ifdef _WIN32
        compile_cmd = "gcc -O2 -o " + exe_file + " " + c_file;
    #endif

    int result = system(compile_cmd.c_str());

    if (result != 0) {
        std::cerr << "[vlc] Native compilation failed." << std::endl;
        return 1;
    }

    std::cout << "[vlc] Built: " << exe_file << std::endl;

    if (run_after) {
        std::cout << "[vlc] Running..." << std::endl;
        std::cout << "---" << std::endl;

        #ifdef _WIN32
            std::string run_cmd = ".\\" + exe_file;
        #else
            std::string run_cmd = "./" + exe_file;
        #endif

        return system(run_cmd.c_str());
    }

    return 0;
}

void create_project(const std::string& name) {
    fs::create_directories(name + "/src");
    fs::create_directories(name + "/assets");
    fs::create_directories(name + "/tests");

    write_file(name + "/velora.toml",
        "[package]\n"
        "name = \"" + name + "\"\n"
        "version = \"0.1.0\"\n"
        "edition = \"2026\"\n"
        "\n"
        "[dependencies]\n"
        "\n"
        "[veloragame]\n"
        "backend = \"Auto\"\n"
        "vsync = true\n"
    );

    write_file(name + "/src/main.vel",
        "// " + name + " — built with Velora\n"
        "\n"
        "main:\n"
        "    print(\"Hello from " + name + "!\")\n"
    );

    std::cout << "Created new Velora project: " << name << "/" << std::endl;
    std::cout << "  " << name << "/velora.toml" << std::endl;
    std::cout << "  " << name << "/src/main.vel" << std::endl;
    std::cout << "  " << name << "/assets/" << std::endl;
    std::cout << "  " << name << "/tests/" << std::endl;
    std::cout << std::endl;
    std::cout << "Run it with: velora run " << name << "/src/main.vel" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    std::string command = argv[1];

    if (command == "version" || command == "--version" || command == "-v") {
        print_version();
        return 0;
    }

    if (command == "help" || command == "--help" || command == "-h") {
        print_help();
        return 0;
    }

    if (command == "new") {
        if (argc < 3) {
            std::cerr << "Usage: velora new <project-name>" << std::endl;
            return 1;
        }
        create_project(argv[2]);
        return 0;
    }

    if (command == "tokens") {
        if (argc < 3) {
            std::cerr << "Usage: velora tokens <file.vel>" << std::endl;
            return 1;
        }
        std::string source = read_file(argv[2]);
        print_tokens(source);
        return 0;
    }

    if (command == "check") {
        if (argc < 3) {
            std::cerr << "Usage: velora check <file.vel>" << std::endl;
            return 1;
        }
        std::string source = read_file(argv[2]);
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();
        SemanticAnalyzer analyzer;
        bool ok = analyzer.analyze(ast);

        for (auto& w : analyzer.get_warnings()) {
            std::cerr << "  warning [" << w.line << "]: " << w.message << std::endl;
        }
        for (auto& e : analyzer.get_errors()) {
            std::cerr << "  error [" << e.line << "]: " << e.message << std::endl;
        }

        if (ok) {
            std::cout << "No errors found." << std::endl;
        }
        return ok ? 0 : 1;
    }

    if (command == "build") {
        if (argc < 3) {
            std::cerr << "Usage: velora build <file.vel>" << std::endl;
            return 1;
        }
        return compile_and_run(argv[2], false);
    }

    if (command == "run") {
        if (argc < 3) {
            std::cerr << "Usage: velora run <file.vel>" << std::endl;
            return 1;
        }
        return compile_and_run(argv[2], true);
    }

    // If just a file is passed, run it
    if (command.size() > 4 && command.substr(command.size() - 4) == ".vel") {
        return compile_and_run(command, true);
    }

    std::cerr << "Unknown command: " << command << std::endl;
    print_help();
    return 1;
}
