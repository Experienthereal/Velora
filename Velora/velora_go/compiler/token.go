package compiler

type TokenType string

const (
	TUnknown    TokenType = "UNKNOWN"
	TEOF        TokenType = "EOF"
	TNewline    TokenType = "NEWLINE"
	TIndent     TokenType = "INDENT"
	TDedent     TokenType = "DEDENT"
	TIdentifier TokenType = "IDENTIFIER"
	TInteger    TokenType = "INTEGER"
	TFloat      TokenType = "FLOAT"
	TString     TokenType = "STRING"
	TFString    TokenType = "FSTRING"
	TColorHex   TokenType = "COLOR_HEX"

	TPlus      TokenType = "+"
	TMinus     TokenType = "-"
	TStar      TokenType = "*"
	TSlash     TokenType = "/"
	TPercent   TokenType = "%"
	TStarStar  TokenType = "**"

	TEq        TokenType = "=="
	TNeq       TokenType = "!="
	TLess      TokenType = "<"
	TGreater   TokenType = ">"
	TLeq       TokenType = "<="
	TGeq       TokenType = ">="

	TAssign    TokenType = "="
	TWalrus    TokenType = ":="
	TConstDecl TokenType = "::"
	TColon     TokenType = ":"
	TComma     TokenType = ","
	TDot       TokenType = "."
	TArrow     TokenType = "->"

	TLParen    TokenType = "("
	TRParen    TokenType = ")"
	TLBracket  TokenType = "["
	TRBracket  TokenType = "]"
	TLBrace    TokenType = "{"
	TRBrace    TokenType = "}"

	// Keywords
	KIf        TokenType = "if"
	KElse      TokenType = "else"
	KElif      TokenType = "elif"
	KWhile     TokenType = "while"
	KFor       TokenType = "for"
	KIn        TokenType = "in"
	KDef       TokenType = "def"
	KFunc      TokenType = "func"
	KReturn    TokenType = "return"
	KStruct    TokenType = "struct"
	KClass     TokenType = "class"
	KTrait     TokenType = "trait"
	KImpl      TokenType = "impl"
	KMatch     TokenType = "match"
	KBreak     TokenType = "break"
	KContinue  TokenType = "continue"
	KAnd       TokenType = "and"
	KOr        TokenType = "or"
	KNot       TokenType = "not"
	KTrue      TokenType = "true"
	KFalse     TokenType = "false"
	KNull      TokenType = "null"
)

type Token struct {
	Type   TokenType
	Value  string
	Line   int
	Column int
}
