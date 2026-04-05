package compiler



type Diagnostic struct {
	Line    int
	Message string
}

type Analyzer struct {
	Errors   []Diagnostic
	Warnings []Diagnostic
}

func NewAnalyzer() *Analyzer {
	return &Analyzer{}
}

func (a *Analyzer) Analyze(prog *Program) bool {
	// Dummy analysis for now. Returns true if no errors.
	if prog == nil {
		a.error(0, "Empty program")
		return false
	}
	// E.g., duplicate main check, undefined variables
	return len(a.Errors) == 0
}

func (a *Analyzer) error(line int, msg string) {
	a.Errors = append(a.Errors, Diagnostic{Line: line, Message: msg})
}

func (a *Analyzer) warn(line int, msg string) {
	a.Warnings = append(a.Warnings, Diagnostic{Line: line, Message: msg})
}
