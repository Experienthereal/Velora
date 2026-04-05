/*
 * compiler_bridge.cpp
 * ───────────────────
 * Exposes the Velora compiler pipeline as velora_main()
 * so the installer exe can call it directly when invoked
 * as a compiler command (velora run, velora build, etc.)
 *
 * All compiler source files are #included here so the
 * entire compiler becomes one compilation unit — making
 * a single, portable, zero-dependency exe possible.
 */

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <direct.h>

// ── Pull in every compiler module as one unit ──────────────────
// (token.h is header-only — no .cpp)
#include "../compiler/token.h"
#include "../compiler/lexer.cpp"
#include "../compiler/parser.cpp"
#include "../compiler/analyzer.cpp"
#include "../compiler/codegen.cpp"
// ───────────────────────────────────────────────────────────────

// Path helpers (no std::filesystem needed)
static std::string vb_stem(const std::string& path) {
    // extract filename stem: "foo/bar/main.vel" → "main"
    size_t slash = path.find_last_of("/\\");
    std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    size_t dot = name.rfind('.');
    return (dot == std::string::npos) ? name : name.substr(0, dot);
}

static void vb_mkdir(const std::string& dir) {
#ifdef _WIN32
    _mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0755);
#endif
}

// ─── Helpers (same as compiler/main.cpp but no WinMain conflict) ───

static std::string vb_read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "velora: cannot open '" << path << "'" << std::endl;
        return "";
    }
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

static void vb_write_file(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (f.is_open()) f << content;
}

static void vb_print_version() {
    std::cout << "Velora 0.1.0\n";
    std::cout << "The Velora Programming Language\n";
    std::cout << "https://velora.dev\n";
}

static void vb_print_help() {
    std::cout << "Velora Programming Language v0.1.0\n\n";
    std::cout << "Usage:\n";
    std::cout << "  velora run   <file.vel>    Build and run a program\n";
    std::cout << "  velora build <file.vel>    Compile to native executable\n";
    std::cout << "  velora check <file.vel>    Check for errors only\n";
    std::cout << "  velora tokens <file.vel>   Show token stream\n";
    std::cout << "  velora new   <name>        Create a new project\n";
    std::cout << "  velora version             Show version\n";
    std::cout << "  velora help                Show this help\n";
}

static void vb_print_tokens(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    for (auto& t : tokens) {
        if (t.type == TokenType::NEWLINE || t.type == TokenType::END_OF_FILE) continue;
        std::string kind;
        if      (t.type == TokenType::INTEGER)    kind = "INT";
        else if (t.type == TokenType::FLOAT)      kind = "FLOAT";
        else if (t.type == TokenType::STRING)     kind = "STRING";
        else if (t.type == TokenType::FSTRING)    kind = "FSTRING";
        else if (t.type == TokenType::IDENTIFIER) kind = "IDENT";
        else if (t.type == TokenType::WALRUS)     kind = "WALRUS";
        else if (t.type == TokenType::CONST_DECL) kind = "CONST";
        else if (t.type == TokenType::INDENT)     kind = "INDENT";
        else if (t.type == TokenType::DEDENT)     kind = "DEDENT";
        else if (t.type == TokenType::COLOR_HEX)  kind = "COLOR";
        else kind = "TOK";
        std::cout << "  [" << t.line << ":" << t.column << "] "
                  << kind << " = '" << t.value << "'\n";
    }
}

static int vb_compile(const std::string& path, bool run_after) {
    std::string source = vb_read_file(path);
    if (source.empty()) return 1;

    std::cout << "[vlc] Lexing..." << std::endl;
    Lexer lexer(source);
    std::vector<Token> tokens;
    try { tokens = lexer.tokenize(); }
    catch (std::exception& e) {
        std::cerr << "  lex error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "[vlc] Parsing..." << std::endl;
    Parser parser(tokens);
    ASTPtr ast;
    try { ast = parser.parse(); }
    catch (std::exception& e) {
        std::cerr << "  parse error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "[vlc] Analyzing..." << std::endl;
    SemanticAnalyzer analyzer;
    bool ok = analyzer.analyze(ast);
    for (auto& w : analyzer.get_warnings())
        std::cerr << "  warning [" << w.line << "]: " << w.message << "\n";
    for (auto& e2 : analyzer.get_errors())
        std::cerr << "  error   [" << e2.line << "]: " << e2.message << "\n";
    if (!ok) {
        std::cerr << "[vlc] Failed (" << analyzer.get_errors().size() << " error(s))\n";
        return 1;
    }

    std::cout << "[vlc] Generating C code..." << std::endl;
    CodeGenerator cg;
    std::string c_code = cg.generate(ast);

    std::string base = vb_stem(path);
    std::string c_file = base + ".c";
    std::string exe_file = base;
#ifdef _WIN32
    exe_file += ".exe";
#endif
    vb_write_file(c_file, c_code);
    std::cout << "[vlc] Compiling native binary..." << std::endl;

#ifdef _WIN32
    std::string compile_cmd = "clang -O2 -o " + exe_file + " " + c_file + " 2>&1";
#else
    std::string compile_cmd = "clang -O2 -o " + exe_file + " " + c_file + " -lm 2>&1";
#endif

    int res = system(compile_cmd.c_str());
    if (res != 0) {
        std::cerr << "[vlc] C compilation failed. Is Clang installed?\n";
        std::cerr << "      Please install LLVM / Clang for Windows.\n";
        return 1;
    }
    std::cout << "[vlc] Built: " << exe_file << "\n";

    if (run_after) {
        std::cout << "[vlc] Running...\n---\n";
#ifdef _WIN32
        return system((".\\" + exe_file).c_str());
#else
        return system(("./" + exe_file).c_str());
#endif
    }
    return 0;
}

static void vb_create_project(const std::string& name) {
    vb_mkdir(name);
    vb_mkdir(name + "/src");
    vb_mkdir(name + "/assets");
    vb_mkdir(name + "/tests");

    vb_write_file(name + "/velora.toml",
        "[package]\nname = \"" + name + "\"\nversion = \"0.1.0\"\n\n[veloragame]\nbackend = \"Auto\"\n");
    vb_write_file(name + "/src/main.vel",
        "// " + name + " — built with Velora\n\nmain:\n    print(\"Hello from " + name + "!\")\n");

    std::cout << "Created project: " << name << "/\n";
    std::cout << "  " << name << "/velora.toml\n";
    std::cout << "  " << name << "/src/main.vel\n\n";
    std::cout << "Run: velora run " << name << "/src/main.vel\n";
}

// ─── Public entry point called by the installer EXE ───────────

int velora_main(int argc, char** argv) {
    if (argc < 2) { vb_print_help(); return 0; }

    std::string cmd = argv[1];

    if (cmd == "version" || cmd == "--version" || cmd == "-v") {
        vb_print_version(); return 0;
    }
    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        vb_print_help(); return 0;
    }
    if (cmd == "tokens" && argc >= 3) {
        std::string src = vb_read_file(argv[2]);
        if (!src.empty()) vb_print_tokens(src);
        return 0;
    }
    if (cmd == "check" && argc >= 3) {
        std::string source = vb_read_file(argv[2]);
        if (source.empty()) return 1;
        Lexer lx(source); auto toks = lx.tokenize();
        Parser pr(toks); auto ast = pr.parse();
        SemanticAnalyzer an; bool ok = an.analyze(ast);
        for (auto& w : an.get_warnings())
            std::cerr << "  warning [" << w.line << "]: " << w.message << "\n";
        for (auto& e : an.get_errors())
            std::cerr << "  error   [" << e.line << "]: " << e.message << "\n";
        if (ok) std::cout << "No errors.\n";
        return ok ? 0 : 1;
    }
    if (cmd == "build" && argc >= 3) return vb_compile(argv[2], false);
    if (cmd == "run"   && argc >= 3) return vb_compile(argv[2], true);
    if (cmd == "new"   && argc >= 3) { vb_create_project(argv[2]); return 0; }

    // Direct file execution: velora main.vel
    if (cmd.size() > 4 && cmd.substr(cmd.size() - 4) == ".vel") {
        return vb_compile(cmd, true);
    }

    std::cerr << "velora: unknown command '" << cmd << "'\n";
    vb_print_help();
    return 1;
}
