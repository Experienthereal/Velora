package main

import (
	"fmt"
	"os"
	"strings"

	"velora/compiler"
)

// Entry point — detects mode and routes accordingly
func main() {
	args := os.Args

	// No args = installer UI
	if len(args) < 2 {
		runInstallerUI()
		return
	}

	cmd := strings.ToLower(strings.TrimSpace(args[1]))

	switch cmd {
	case "--tray":
		runTray()
		return
	case "--uninstall":
		runUninstallUI()
		return
	case "--install":
		runInstallerUI()
		return
	}

	// Compiler commands — need a console
	attachConsole()

	switch cmd {
	case "version", "--version", "-v":
		fmt.Println("Velora 0.1.0")
		fmt.Println("The Velora Programming Language")
		fmt.Println("https://velora.dev")

	case "help", "--help", "-h":
		printHelp()

	case "run":
		if len(args) < 3 {
			fmt.Fprintln(os.Stderr, "Usage: velora run <file.vel>")
			os.Exit(1)
		}
		os.Exit(runFile(args[2], true))

	case "build", "built":
		if len(args) < 3 {
			fmt.Fprintln(os.Stderr, "Usage: velora build <file.vel>")
			os.Exit(1)
		}
		os.Exit(runFile(args[2], false))

	case "check":
		if len(args) < 3 {
			fmt.Fprintln(os.Stderr, "Usage: velora check <file.vel>")
			os.Exit(1)
		}
		os.Exit(checkFile(args[2]))

	case "tokens":
		if len(args) < 3 {
			fmt.Fprintln(os.Stderr, "Usage: velora tokens <file.vel>")
			os.Exit(1)
		}
		showTokens(args[2])

	case "new":
		if len(args) < 3 {
			fmt.Fprintln(os.Stderr, "Usage: velora new <project>")
			os.Exit(1)
		}
		createProject(args[2])

	default:
		// Direct file: velora main.vel
		if strings.HasSuffix(cmd, ".vel") {
			os.Exit(runFile(cmd, true))
		}
		fmt.Fprintf(os.Stderr, "velora: unknown command '%s'\n", cmd)
		printHelp()
		os.Exit(1)
	}
}

func printHelp() {
	fmt.Print(`Velora Programming Language v0.1.0

Usage:
  velora run   <file.vel>    Compile and run a program
  velora build <file.vel>    Build a native executable
  velora check <file.vel>    Check for errors only
  velora tokens <file.vel>   Show token stream
  velora new   <name>        Create a new project
  velora version             Show version
  velora help                Show this help
`)
}

func runFile(path string, runAfter bool) int {
	src, err := os.ReadFile(path)
	if err != nil {
		fmt.Fprintf(os.Stderr, "velora: cannot open '%s': %v\n", path, err)
		return 1
	}

	fmt.Println("[vlc] Lexing...")
	lexer := compiler.NewLexer(string(src))
	tokens, lexErr := lexer.Tokenize()
	if lexErr != nil {
		fmt.Fprintf(os.Stderr, "  lex error: %v\n", lexErr)
		return 1
	}

	fmt.Println("[vlc] Parsing...")
	parser := compiler.NewParser(tokens)
	ast, parseErr := parser.Parse()
	if parseErr != nil {
		fmt.Fprintf(os.Stderr, "  parse error: %v\n", parseErr)
		return 1
	}

	fmt.Println("[vlc] Analyzing...")
	analyzer := compiler.NewAnalyzer()
	if !analyzer.Analyze(ast) {
		for _, w := range analyzer.Warnings {
			fmt.Fprintf(os.Stderr, "  warning [%d]: %s\n", w.Line, w.Message)
		}
		for _, e := range analyzer.Errors {
			fmt.Fprintf(os.Stderr, "  error   [%d]: %s\n", e.Line, e.Message)
		}
		fmt.Fprintf(os.Stderr, "[vlc] Failed (%d error(s))\n", len(analyzer.Errors))
		return 1
	}
	for _, w := range analyzer.Warnings {
		fmt.Fprintf(os.Stderr, "  warning [%d]: %s\n", w.Line, w.Message)
	}

	fmt.Println("[vlc] Interpreting...")
	interpreter := compiler.NewInterpreter()
	errInterpreter := interpreter.Interpret(ast)
	if errInterpreter != nil {
		fmt.Fprintf(os.Stderr, "  runtime error: %v\n", errInterpreter)
		return 1
	}

	return 0
}

func checkFile(path string) int {
	src, err := os.ReadFile(path)
	if err != nil {
		fmt.Fprintf(os.Stderr, "velora: cannot open '%s': %v\n", path, err)
		return 1
	}

	lexer := compiler.NewLexer(string(src))
	tokens, lexErr := lexer.Tokenize()
	if lexErr != nil {
		fmt.Fprintf(os.Stderr, "  lex error: %v\n", lexErr)
		return 1
	}

	parser := compiler.NewParser(tokens)
	ast, parseErr := parser.Parse()
	if parseErr != nil {
		fmt.Fprintf(os.Stderr, "  parse error: %v\n", parseErr)
		return 1
	}

	analyzer := compiler.NewAnalyzer()
	ok := analyzer.Analyze(ast)
	for _, w := range analyzer.Warnings {
		fmt.Fprintf(os.Stderr, "  warning [%d]: %s\n", w.Line, w.Message)
	}
	for _, e := range analyzer.Errors {
		fmt.Fprintf(os.Stderr, "  error   [%d]: %s\n", e.Line, e.Message)
	}
	if ok {
		fmt.Println("No errors found.")
	}
	return map[bool]int{true: 0, false: 1}[ok]
}

func showTokens(path string) {
	src, err := os.ReadFile(path)
	if err != nil {
		fmt.Fprintf(os.Stderr, "velora: cannot open '%s': %v\n", path, err)
		return
	}
	lexer := compiler.NewLexer(string(src))
	tokens, _ := lexer.Tokenize()
	for _, t := range tokens {
		if t.Type == compiler.TNewline || t.Type == compiler.TEOF {
			continue
		}
		fmt.Printf("  [%d:%d] %s = '%s'\n", t.Line, t.Column, t.Type, t.Value)
	}
}

func createProject(name string) {
	dirs := []string{name, name + "/src", name + "/assets", name + "/tests"}
	for _, d := range dirs {
		os.MkdirAll(d, 0755)
	}

	toml := "[package]\nname = \"" + name + "\"\nversion = \"0.1.0\"\n\n[veloragame]\nbackend = \"Auto\"\n"
	os.WriteFile(name+"/velora.toml", []byte(toml), 0644)

	mainVel := "// " + name + " — built with Velora\n\nmain:\n    print(\"Hello from " + name + "!\")\n"
	os.WriteFile(name+"/src/main.vel", []byte(mainVel), 0644)

	fmt.Printf("Created project: %s/\n", name)
	fmt.Printf("  %s/velora.toml\n", name)
	fmt.Printf("  %s/src/main.vel\n\n", name)
	fmt.Printf("Run: velora run %s/src/main.vel\n", name)
}
