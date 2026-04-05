package compiler

import (
	"fmt"
	"unicode"
)

type Lexer struct {
	src    string
	pos    int
	line   int
	col    int
	indents []int
}

func NewLexer(src string) *Lexer {
	return &Lexer{
		src:    src,
		pos:    0,
		line:   1,
		col:    1,
		indents: []int{0},
	}
}

func (l *Lexer) current() byte {
	if l.pos >= len(l.src) {
		return 0
	}
	return l.src[l.pos]
}

func (l *Lexer) peek(offset int) byte {
	if l.pos+offset >= len(l.src) {
		return 0
	}
	return l.src[l.pos+offset]
}

func (l *Lexer) advance() byte {
	c := l.current()
	l.pos++
	if c == '\n' {
		l.line++
		l.col = 1
	} else {
		l.col++
	}
	return c
}

func (l *Lexer) match(expected string) bool {
	if l.pos+len(expected) > len(l.src) {
		return false
	}
	if l.src[l.pos:l.pos+len(expected)] == expected {
		for i := 0; i < len(expected); i++ {
			l.advance()
		}
		return true
	}
	return false
}

func (l *Lexer) skipWhitespace() {
	for {
		c := l.current()
		if c == ' ' || c == '\t' || c == '\r' {
			l.advance()
		} else if c == '/' && l.peek(1) == '/' {
			for l.current() != '\n' && l.current() != 0 {
				l.advance()
			}
		} else {
			break
		}
	}
}

func (l *Lexer) Tokenize() ([]Token, error) {
	var tokens []Token

	for l.pos < len(l.src) {
		// Handle Indentation at the start of a line
		if l.col == 1 {
			spaces := 0
			for l.current() == ' ' {
				spaces++
				l.advance()
			}
			if l.current() == '\n' || l.current() == '\r' || l.current() == 0 {
				continue // empty line
			}
			if l.current() == '/' && l.peek(1) == '/' {
				continue // comment line
			}

			lastIndent := l.indents[len(l.indents)-1]
			if spaces > lastIndent {
				l.indents = append(l.indents, spaces)
				tokens = append(tokens, Token{Type: TIndent, Value: "", Line: l.line, Column: 1})
			} else if spaces < lastIndent {
				for len(l.indents) > 1 && l.indents[len(l.indents)-1] > spaces {
					l.indents = l.indents[:len(l.indents)-1]
					tokens = append(tokens, Token{Type: TDedent, Value: "", Line: l.line, Column: 1})
				}
				if l.indents[len(l.indents)-1] != spaces {
					return nil, fmt.Errorf("indentation error at %d:%d", l.line, l.col)
				}
			}
		}

		l.skipWhitespace()
		if l.pos >= len(l.src) {
			break
		}

		c := l.current()
		line, col := l.line, l.col

		if c == '\n' {
			tokens = append(tokens, Token{Type: TNewline, Value: "\\n", Line: line, Column: col})
			l.advance()
			continue
		}

		if unicode.IsLetter(rune(c)) || c == '_' {
			start := l.pos
			for unicode.IsLetter(rune(l.current())) || unicode.IsDigit(rune(l.current())) || l.current() == '_' {
				l.advance()
			}
			val := l.src[start:l.pos]

			keywords := map[string]TokenType{
				"if": KIf, "else": KElse, "elif": KElif, "while": KWhile, "for": KFor,
				"in": KIn, "def": KDef, "func": KFunc, "return": KReturn, "struct": KStruct,
				"class": KClass, "trait": KTrait, "impl": KImpl, "match": KMatch,
				"break": KBreak, "continue": KContinue, "and": KAnd, "or": KOr, "not": KNot,
				"true": KTrue, "false": KFalse, "null": KNull,
			}

			if kw, ok := keywords[val]; ok {
				if l.current() == '"' && val == "f" {
					// F-string
					l.advance()
					start = l.pos
					for l.current() != '"' && l.current() != 0 {
						l.advance()
					}
					if l.current() == '"' { l.advance() }
					tokens = append(tokens, Token{Type: TFString, Value: l.src[start:l.pos-1], Line: line, Column: col})
				} else {
					tokens = append(tokens, Token{Type: kw, Value: val, Line: line, Column: col})
				}
			} else {
				tokens = append(tokens, Token{Type: TIdentifier, Value: val, Line: line, Column: col})
			}
			continue
		}

		if unicode.IsDigit(rune(c)) {
			start := l.pos
			isFloat := false
			for unicode.IsDigit(rune(l.current())) || l.current() == '.' {
				if l.current() == '.' {
					if isFloat { break }
					isFloat = true
				}
				l.advance()
			}
			val := l.src[start:l.pos]
			tType := TInteger
			if isFloat {
				tType = TFloat
			}
			tokens = append(tokens, Token{Type: tType, Value: val, Line: line, Column: col})
			continue
		}

		if c == '"' {
			l.advance()
			start := l.pos
			for l.current() != '"' && l.current() != 0 {
				l.advance()
			}
			val := l.src[start:l.pos]
			if l.current() == '"' { l.advance() }
			tokens = append(tokens, Token{Type: TString, Value: val, Line: line, Column: col})
			continue
		}
		
		if c == '#' {
			l.advance()
			start := l.pos
			for i:=0; i<6; i++ {
				if unicode.IsDigit(rune(l.current())) || (l.current() >= 'a' && l.current() <= 'f') || (l.current() >= 'A' && l.current() <= 'F') {
					l.advance()
				} else {
					break
				}
			}
			val := l.src[start:l.pos]
			tokens = append(tokens, Token{Type: TColorHex, Value: val, Line: line, Column: col})
			continue
		}

		// Operators
		ops := []struct {
			str string
			typ TokenType
		}{
			{":=", TWalrus}, {"::", TConstDecl}, {"==", TEq}, {"!=", TNeq},
			{"<=", TLeq}, {">=", TGeq}, {"->", TArrow}, {"**", TStarStar},
			{"+", TPlus}, {"-", TMinus}, {"*", TStar}, {"/", TSlash},
			{"%", TPercent}, {"<", TLess}, {">", TGreater}, {"=", TAssign},
			{":", TColon}, {",", TComma}, {".", TDot},
			{"(", TLParen}, {")", TRParen}, {"[", TLBracket}, {"]", TRBracket},
			{"{", TLBrace}, {"}", TRBrace},
		}

		matched := false
		for _, op := range ops {
			if l.match(op.str) {
				tokens = append(tokens, Token{Type: op.typ, Value: op.str, Line: line, Column: col})
				matched = true
				break
			}
		}

		if !matched {
			return nil, fmt.Errorf("unexpected character '%c' at %d:%d", c, line, col)
		}
	}

	for len(l.indents) > 1 {
		l.indents = l.indents[:len(l.indents)-1]
		tokens = append(tokens, Token{Type: TDedent, Value: "", Line: l.line, Column: 1})
	}

	tokens = append(tokens, Token{Type: TEOF, Value: "", Line: l.line, Column: l.col})
	return tokens, nil
}
