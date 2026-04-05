package compiler

import (
	"fmt"
	"strconv"
)

type Environment struct {
	Variables map[string]interface{}
	Parent    *Environment
}

func NewEnvironment(parent *Environment) *Environment {
	return &Environment{
		Variables: make(map[string]interface{}),
		Parent:    parent,
	}
}

func (e *Environment) Define(name string, value interface{}) {
	e.Variables[name] = value
}

func (e *Environment) Get(name string) (interface{}, bool) {
	val, ok := e.Variables[name]
	if ok {
		return val, true
	}
	if e.Parent != nil {
		return e.Parent.Get(name)
	}
	return nil, false
}

func (e *Environment) Assign(name string, value interface{}) bool {
	if _, ok := e.Variables[name]; ok {
		e.Variables[name] = value
		return true
	}
	if e.Parent != nil {
		return e.Parent.Assign(name, value)
	}
	return false
}

type Interpreter struct {
	Globals *Environment
}

func NewInterpreter() *Interpreter {
	globals := NewEnvironment(nil)
	// Add built-ins
	globals.Define("print", func(args []interface{}) interface{} {
		for i, arg := range args {
			if i > 0 {
				fmt.Print(" ")
			}
			fmt.Print(arg)
		}
		fmt.Println()
		return nil
	})
	return &Interpreter{Globals: globals}
}

func (i *Interpreter) Interpret(prog *Program) error {
	if prog == nil {
		return nil
	}
	for _, stmt := range prog.Body {
		_, err := i.evaluate(stmt, i.Globals)
		if err != nil {
			return err
		}
	}
	return nil
}

func (i *Interpreter) evaluate(node ASTNode, env *Environment) (interface{}, error) {
	switch n := node.(type) {
	case *VarDecl:
		val, err := i.evaluate(n.Value, env)
		if err != nil {
			return nil, err
		}
		env.Define(n.Name, val)
		return nil, nil

	case *Assignment:
		val, err := i.evaluate(n.Value, env)
		if err != nil {
			return nil, err
		}
		if !env.Assign(n.Name, val) {
			return nil, fmt.Errorf("undefined variable '%s'", n.Name)
		}
		return nil, nil

	case *CallExpr:
		fn, ok := env.Get(n.Name)
		if !ok {
			return nil, fmt.Errorf("undefined function '%s'", n.Name)
		}
		
		var args []interface{}
		for _, argExpr := range n.Args {
			argVal, err := i.evaluate(argExpr, env)
			if err != nil {
				return nil, err
			}
			args = append(args, argVal)
		}

		if builtin, isBuiltin := fn.(func([]interface{}) interface{}); isBuiltin {
			return builtin(args), nil
		}

		if decl, isDecl := fn.(*FunctionDecl); isDecl {
			closure := NewEnvironment(env)
			for i, argName := range decl.Args {
				if i < len(args) {
					closure.Define(argName, args[i])
				}
			}
			for _, stmt := range decl.Body {
				// handle return statements properly in a full version
				_, err := i.evaluate(stmt, closure)
				if err != nil {
					return nil, err
				}
			}
			return nil, nil
		}

		return nil, fmt.Errorf("'%s' is not callable", n.Name)

	case *Expression:
		// Attempt to resolve as variable
		if val, ok := env.Get(n.Value); ok {
			return val, nil
		}
		// Try string literal
		if len(n.Value) >= 2 && n.Value[0] == '"' && n.Value[len(n.Value)-1] == '"' {
			return n.Value[1 : len(n.Value)-1], nil
		}
		// Try int
		if intVal, err := strconv.Atoi(n.Value); err == nil {
			return intVal, nil
		}
		// Try float
		if floatVal, err := strconv.ParseFloat(n.Value, 64); err == nil {
			return floatVal, nil
		}
		return n.Value, nil // Fallback generic string/ident
		
	case *FunctionDecl:
		env.Define(n.Name, n)
		return nil, nil

	}
	return nil, nil
}
