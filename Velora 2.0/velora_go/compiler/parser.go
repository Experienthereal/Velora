package compiler

import "fmt"

// Simple dummy AST nodes for now, representing structure.
type ASTNode interface{}

type Program struct {
	Body []ASTNode
}

type VarDecl struct {
	Name    string
	IsConst bool
	Value   ASTNode
}

type Assignment struct {
	Name  string
	Value ASTNode
}

type FunctionDecl struct {
	Name string
	Args []string
	Body []ASTNode
}

type IfStmt struct {
	Cond ASTNode
	Body []ASTNode
}

type WhileStmt struct {
	Cond ASTNode
	Body []ASTNode
}

type ReturnStmt struct {
	Value ASTNode
}

type Expression struct {
	Value string
}

type CallExpr struct {
	Name string
	Args []ASTNode
}

// Very basic parser just to get things going, similar to the C++ initial version.
type Parser struct {
	tokens []Token
	pos    int
}

func NewParser(tokens []Token) *Parser {
	return &Parser{tokens: tokens, pos: 0}
}

func (p *Parser) current() Token {
	if p.pos >= len(p.tokens) {
		return p.tokens[len(p.tokens)-1]
	}
	return p.tokens[p.pos]
}

func (p *Parser) advance() Token {
	t := p.current()
	p.pos++
	return t
}

func (p *Parser) match(types ...TokenType) bool {
	for _, t := range types {
		if p.current().Type == t {
			p.advance()
			return true
		}
	}
	return false
}

func (p *Parser) Parse() (*Program, error) {
	prog := &Program{}
	for p.current().Type != TEOF {
		if p.current().Type == TNewline {
			p.advance()
			continue
		}
		
		stmt, err := p.parseStmt()
		if err != nil {
			return nil, err
		}
		if stmt != nil {
			prog.Body = append(prog.Body, stmt)
		}
	}
	return prog, nil
}

func (p *Parser) parseStmt() (ASTNode, error) {
	t := p.current()

	if t.Type == KFunc || t.Type == KDef {
		p.advance()
		if p.current().Type != TIdentifier {
			return nil, fmt.Errorf("expected function name")
		}
		name := p.advance().Value
		p.match(TLParen)
		// args...
		for p.current().Type != TRParen && p.current().Type != TEOF {
			p.advance()
		}
		p.match(TRParen)
		p.match(TArrow, TIdentifier) // optional return type
		p.match(TColon)
		p.match(TNewline)
		p.match(TIndent)
		body, _ := p.parseBlock()
		return &FunctionDecl{Name: name, Body: body}, nil
	}

	if t.Type == TIdentifier {
		name := p.advance().Value
		if p.current().Type == TWalrus {
			p.advance()
			expr := p.parseExpr()
			return &VarDecl{Name: name, IsConst: false, Value: expr}, nil
		} else if p.current().Type == TConstDecl {
			p.advance()
			expr := p.parseExpr()
			return &VarDecl{Name: name, IsConst: true, Value: expr}, nil
		} else if p.current().Type == TAssign {
			p.advance()
			expr := p.parseExpr()
			return &Assignment{Name: name, Value: expr}, nil
		} else if p.current().Type == TLParen {
			// function call
			p.advance()
			for p.current().Type != TRParen && p.current().Type != TEOF {
				p.advance() // fast forward args
			}
			p.match(TRParen)
			return &CallExpr{Name: name}, nil
		}
		// otherwise just identifier expression
		return &Expression{Value: name}, nil
	}
	
	// Skip unhandled tokens for now to prevent infinite loop
	p.advance()
	return nil, nil
}

func (p *Parser) parseBlock() ([]ASTNode, error) {
	var body []ASTNode
	for p.current().Type != TDedent && p.current().Type != TEOF {
		if p.current().Type == TNewline {
			p.advance()
			continue
		}
		stmt, _ := p.parseStmt()
		if stmt != nil {
			body = append(body, stmt)
		}
	}
	p.match(TDedent)
	return body, nil
}

func (p *Parser) parseExpr() ASTNode {
	// Dummy expression parser
	val := p.current().Value
	p.advance()
	return &Expression{Value: val}
}
