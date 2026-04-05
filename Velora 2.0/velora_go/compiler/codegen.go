package compiler

import (
	"fmt"
	"strings"
)

type CodeGen struct {
	imports []string
}

func NewCodeGen() *CodeGen {
	return &CodeGen{
		imports: []string{"fmt"},
	}
}

func (cg *CodeGen) Generate(prog *Program) string {
	var builder strings.Builder

	builder.WriteString("package main\n\n")
	
	// Default imports
	builder.WriteString("import (\n")
	for _, imp := range cg.imports {
		builder.WriteString(fmt.Sprintf("\t\"%s\"\n", imp))
	}
	builder.WriteString(")\n\n")

	// Render nodes
	if prog != nil {
		for _, node := range prog.Body {
			builder.WriteString(cg.generateNode(node))
			builder.WriteString("\n")
		}
	}

	return builder.String()
}

func (cg *CodeGen) generateNode(n ASTNode) string {
	switch v := n.(type) {
	case *FunctionDecl:
		body := ""
		for _, stmt := range v.Body {
			body += "\t" + cg.generateNode(stmt) + "\n"
		}
		args := strings.Join(v.Args, ", ")
		return fmt.Sprintf("func %s(%s) {\n%s}\n", v.Name, args, body)
	
	case *VarDecl:
		op := ":="
		return fmt.Sprintf("%s %s %s", v.Name, op, cg.generateNode(v.Value))
	
	case *Assignment:
		return fmt.Sprintf("%s = %s", v.Name, cg.generateNode(v.Value))

	case *CallExpr:
		args := ""
		for i, a := range v.Args {
			if i > 0 {
				args += ", "
			}
			args += cg.generateNode(a)
		}
		// Map 'print' to 'fmt.Println'
		if v.Name == "print" {
			v.Name = "fmt.Println"
		}
		return fmt.Sprintf("%s(%s)", v.Name, args)
	
	case *Expression:
		return v.Value
		
	default:
		return ""
	}
}
