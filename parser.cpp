#include <assert.h>
#include "namestore.h"
#include "lex.h"
#include "parser.h"
#include "error.h"
#include "code.h"
#include "debugger.h"

//---------------------------------------------------------------
// Token set types (still in C; it's more terse)
//---------------------------------------------------------------

/* Test for membership */

int In(set_t S, int v)
{
	assert(v>=0 && v<MAX_SET_ELTS);
	return ( (S.v[v/32] & (1l << (v%32))) != 0l);
}

/* Union `operator' */

set_t Union(set_t S1, set_t S2)
{
	static set_t S;
	int i;
	for (i=0; i< MAX_SET_LONGS; i++)
		S.v[i] = S1.v[i] | S2.v[i];
	return S;
}

/* Combine `operator' */

set_t Combine(set_t S, ...)
{
	va_list args;
	static set_t R;
	va_start(args, S);
	for (;;)
	{
		int t = (int)va_arg(args, int);
		assert(t>=-1 && t < MAX_SET_ELTS);
		if (t == -1) break;
		S.v[t / 32] |= 1l << (t % 32 );
	}
	va_end(args);
	return R = S;
}

/* Set initialiser */

set_t Set(int e, ...)
{
	va_list args;
	static set_t S;
	int i;
	for (i=0;i<MAX_SET_LONGS;i++) S.v[i] = 0l;
	assert(e>=-1 && e < MAX_SET_ELTS);
	va_start(args, e);
	S.v[e / 32] |= 1l << (e % 32 );
	for (;;)
	{
		int t = va_arg(args, int);
		assert(t>=-1 && t < MAX_SET_ELTS);
		if (t == -1) break;
		S.v[t / 32] |= 1l << (t % 32 );
	}
	va_end(args);
	return S;
}

//------------------------------------------------------------
// Commonly-used type and syntax checking support routines

symtab_entry_t *parser_t::CheckTypes(symtab_entry_t *t1, symtab_entry_t *t2)
{
	if (t1 != t2)
	{
		if (t1 != unknown_type && t2 != unknown_type)
			ErrorHandler->Error(ERR_TYPEWAR);
		return unknown_type;
	}
	return t1;
}

void parser_t::TypeError(symtab_entry_t *e, char *class_name)
{
	if (e->Type() != unknown_type)
		ErrorHandler->Error(ERR_TYPE, class_name);
}

void parser_t::Synchronise(set_t stop)
{
	if (!In(stop, Lex->Token()))
	{
		ErrorHandler->Error(ERR_SYNTAX);
		while (!In(stop, Lex->Token()))
			Lex->NextToken();
	}
}

void parser_t::Expect(token_t t, set_t stop)
{
	if (Lex->Token() != t)
	{
		ErrorHandler->Error(ERR_EXPECT, Lex->TokenID(t));
		while (!In(stop, Lex->Token()))
			Lex->NextToken();
	}
	else Lex->NextToken();
	Synchronise(stop);
}

//-------------------------------------------------------------------
// The parser routines for nonterminals in the grammar

set_t	blockfirst, constfirst, exprfirst, stmtfirst, relops, addops,
	mulops, selectors;

void parser_t::Program()
{
	int vars = Code->NewLabel();
	int temps = Code->NewLabel();
	int start = Code->NewLabel();
	blockfirst = Set(T_BEGIN, T_CONST, T_PROCEDURE, T_TYPE, T_VAR, -1);
	constfirst = Set(T_IDENT, T_INTEGER, -1);
	exprfirst = Set(T_MINUS, T_PLUS, T_NOT, T_LEFTPAREN,
		T_IDENT, T_INTEGER, -1);
	stmtfirst = Set(T_BEGIN, T_IF, T_WHILE, T_IDENT, -1);
	relops = Set(T_EQUALS, T_GREATER, T_LESS, T_NOTEQUAL,
		T_GREATEREQU, T_LESSEQU, -1);
	addops = Set(T_PLUS, T_MINUS, T_OR, -1);
	mulops = Set(T_ASTERISK, T_MOD, T_AND, T_DIV, -1);
	selectors = Set(T_PERIOD, T_LEFTBRACKET, -1);
	Expect(T_PROGRAM,
	     Combine(blockfirst, T_IDENT, T_SEMICOLON, T_PERIOD, T_EOF,-1));
	Ident(Combine(blockfirst, T_SEMICOLON, T_PERIOD, T_EOF,-1));
	Code->Emit(OP_PROGRAM, vars, temps, start);
	Expect(T_SEMICOLON, Combine(blockfirst, T_PERIOD, T_EOF,-1));
	SymTab->NewBlock();
	Block(vars, temps, start, Debugger ? Debugger->ScopeIdx() : 0,
		Set(T_PERIOD, T_EOF,-1));
	Code->Emit(OP_ENDPROG);
	SymTab->ExitBlock();
	Expect(T_PERIOD, Set(T_EOF,-1));
}

void parser_t::Block(int vars, int temps, int start,
	pword_t scopeindex, set_t stop)
{
	Synchronise(Union(blockfirst, stop));
	if (Lex->Token() == T_CONST)
		ConstantDefinitions(Combine(stop,
			T_TYPE, T_VAR, T_PROCEDURE, T_BEGIN, -1));
	if (Lex->Token() == T_TYPE)
		TypeDefinitions(Combine(stop,
			T_VAR, T_PROCEDURE, T_BEGIN, -1));
	int vlen;
	if (Lex->Token() == T_VAR)
		vlen = VariableDefinitions(Combine(stop,
			T_PROCEDURE, T_BEGIN, -1));
	else vlen = 0;
	while (Lex->Token() == T_PROCEDURE)
		ProcedureDefinition(Combine(stop,
			T_PROCEDURE, T_BEGIN, -1));
	Code->Emit(OP_DEFADDR, start);
	Code->Emit(OP_SCOPE, scopeindex);
	CompoundStmt(stop);
	Code->Emit(OP_ENDSCOPE);
	Code->Emit(OP_DEFARG, vars, vlen);
	Code->Emit(OP_DEFARG, temps, SymTab->MaxTemp());
}

void parser_t::StandardBlock()
{
	// Define standard objects
	no_name = NameStore->Lookup("");
	unknown_type = SymTab->Define(new standardtype_t(no_name));
	integer_type = SymTab->Define(new standardtype_t(NameStore->Lookup("Integer")));
	boolean_type = SymTab->Define(new standardtype_t(NameStore->Lookup("Boolean")));
	true_literal = SymTab->Define(new constant_t(NameStore->Lookup("True"),
		boolean_type, 1));
	false_literal= SymTab->Define(new constant_t(NameStore->Lookup("False"),
		boolean_type, 0));
	read_proc_idx = NameStore->Lookup("Read");
	read_procedure = SymTab->Define(new standardproc_t(read_proc_idx));
	write_proc_idx = NameStore->Lookup("Write");
	write_procedure = SymTab->Define(new standardproc_t(write_proc_idx));
}

void parser_t::ConstantDefinitions(set_t stop)
{
	set_t stop2 = Combine(stop, T_IDENT, -1);
	Expect(T_CONST, stop2);
	while (Lex->Token() == T_IDENT)
		ConstantDefinition(stop2);
}

void parser_t::ConstantDefinition(set_t stop)
{
	int id = Ident(
		Combine(Union(stop, constfirst), T_EQUALS, T_SEMICOLON, -1));
	Expect(T_EQUALS,
		Combine(Union(stop, constfirst), T_SEMICOLON, -1));
	int v;
	symtab_entry_t *se;
	Constant(v, se, Combine(stop, T_SEMICOLON, -1));
	SymTab->Define(new constant_t(id, se, v));
	Expect(T_SEMICOLON, stop);
}

void parser_t::TypeDefinitions(set_t stop)
{
	set_t stop2 = Combine(stop, T_IDENT, -1);
	Expect(T_TYPE, stop2);
	while (Lex->Token() == T_IDENT)
		TypeDefinition(stop2);
}

void parser_t::TypeDefinition(set_t stop)
{
	set_t stop2 = Combine(stop, T_SEMICOLON, -1);
	int id = Ident(Combine(stop2, T_EQUALS, T_ARRAY, T_RECORD, -1));
	Expect(T_EQUALS, Combine(stop2, T_ARRAY, T_RECORD, -1));
	if (Lex->Token() == T_ARRAY)
		NewArrayType(id, stop2);
	else if (Lex->Token() == T_RECORD)
		NewRecordType(id, stop2);
	else ErrorHandler->Error(ERR_BADTYPEDEF);
	Expect(T_SEMICOLON, stop);
}

void parser_t::NewArrayType(int id, set_t stop)
{
	Expect(T_ARRAY, Combine(Union(stop, constfirst), T_LEFTBRACKET,
		T_RIGHTBRACKET, T_OF, T_IDENT, -1));
	Expect(T_LEFTBRACKET, Combine(Union(stop, constfirst),
		T_RIGHTBRACKET, T_OF, T_IDENT, -1));
	int lower, upper;
	symtab_entry_t *index_type = IndexRange(lower, upper,
		Combine(stop, T_RIGHTBRACKET, T_OF, T_IDENT, -1));
	Expect(T_RIGHTBRACKET, Combine(stop, T_OF, T_IDENT, -1));
	Expect(T_OF, Combine(stop, T_IDENT, -1));
	symtab_entry_t *elttype = TypeID(stop);
	SymTab->Define(new arraytype_t(id, index_type, elttype,
		lower, upper));
}

symtab_entry_t *parser_t::IndexRange(int &lower, int &upper,
		set_t stop)
{
	symtab_entry_t *typl, *typu;
	Constant(lower, typl,
		Combine(Union(stop, constfirst), T_DOUBLEDOT, -1));
	Expect(T_DOUBLEDOT, Union(stop, constfirst));
	Constant(upper, typu, stop);
	typl = CheckTypes(typl, typu);
	if (lower > upper)
	{
		ErrorHandler->Error(ERR_RANGE);
		lower = upper;
	}
	return typl;
}

void parser_t::NewRecordType(int id, set_t stop)
{
	SymTab->NewBlock();
	Expect(T_RECORD, Combine(stop, T_IDENT, T_END, -1));
	int size;
	symtab_entry_t *lastfield = FieldList(size, Combine(stop, T_END, -1));
	Expect(T_END, stop);
	SymTab->ExitBlock();
	SymTab->Define(new recordtype_t(id, size, lastfield));
}

symtab_entry_t *parser_t::FieldList(int &size, set_t stop)
{
	set_t stop2 = Combine(stop, T_SEMICOLON, -1);
	int cnt = 0;
	symtab_entry_t *last;
	symtab_entry_t *type = RecordSection(last, cnt, stop2);
	size = cnt * type->Size();
	while (Lex->Token() == T_SEMICOLON)
	{
		Expect(T_SEMICOLON, Combine(stop2, T_IDENT, -1));
		cnt = 0;
		type = RecordSection(last, cnt, stop2);
		size += cnt * type->Size();
	}
	last->AllocateAddress(FIELD, size);
	return last;
}

symtab_entry_t *parser_t::RecordSection(symtab_entry_t* &last, int &cnt,
	set_t stop)
{
	symtab_entry_t *vtype, *field;
	int id = Ident(Combine(stop, T_COMMA, T_COLON, -1));
	field = SymTab->Define(new fieldtype_t(id, vtype));
	cnt++;
	if (Lex->Token() == T_COMMA)
	{
		Expect(T_COMMA, Combine(stop, T_IDENT, -1));
		vtype = RecordSection(last, cnt, stop);
	}
	else
	{
		Expect(T_COLON, Combine(stop, T_IDENT, -1));
		vtype = TypeID(stop);
		last = field;
	}
	field->Type(vtype);
	return vtype;
}

int parser_t::VariableDefinitions(set_t stop)
{
	set_t stop2 = Combine(stop, T_IDENT, -1);
	int length = 0;
	symtab_entry_t *last;
	Expect(T_VAR, stop2);
	while (Lex->Token() == T_IDENT)
	{
		int cnt = 0;
		symtab_entry_t *typ = VariableDefinition(last, cnt, stop2);
		length += cnt * typ->Size();
	}
	last->AllocateAddress(VARIABLE, 3 + length);
	return length;
}

symtab_entry_t *parser_t::VariableDefinition(symtab_entry_t* &last,
	int &cnt, set_t stop)
{
	symtab_entry_t *type;
	type = VariableGroup(VARIABLE, last, cnt,
		Combine(stop, T_SEMICOLON, -1));
	Expect(T_SEMICOLON, stop);
	return type;
}

symtab_entry_t *parser_t::TypeID(set_t stop)
{
	symtab_entry_t *se = SymTab->Find(Ident(stop));
	if (se->Class() == ARRAYTYPE || se->Class() == RECORDTYPE ||
		se->Class() == STANDARDTYPE)
		return se;
	else return unknown_type;
}

symtab_entry_t *parser_t::VariableGroup(entry_class_t cls,
	symtab_entry_t* &last, int &cnt, set_t stop)
{
	symtab_entry_t *vtype, *var;
	int id = Ident(Combine(stop, T_COMMA, T_COLON, -1));
	switch(cls)
	{
	case VARIABLE:
		var = SymTab->Define(new variable_t(id));
		break;
	case VALUEPARAM:
		var = SymTab->Define(new valueparam_t(id));
		break;
	case REFPARAM:
		var = SymTab->Define(new refparam_t(id));
		break;
	}
	cnt++;
	if (Lex->Token() == T_COMMA)
	{
		Expect(T_COMMA, Combine(stop, T_IDENT, -1));
		vtype = VariableGroup(cls, last, cnt, stop);
	}
	else
	{
		Expect(T_COLON, Combine(stop, T_IDENT, -1));
		vtype = TypeID(stop);
		last = var;
	}
	var->Type(vtype);
	return vtype;
}

void parser_t::ProcedureDefinition(set_t stop)
{
	set_t stop2 = Union(stop, blockfirst);
	Expect(T_PROCEDURE, Combine(stop2, T_IDENT, T_LEFTPAREN,
		T_SEMICOLON, -1));
	// We must define the procedure before we process it
	// in case it is recursive
	procedure_t *pd = (procedure_t *)SymTab->Define(new procedure_t(
		Ident(Combine(stop2, T_LEFTPAREN,	T_SEMICOLON, -1)),
		Code->NewLabel()));
	SymTab->NewBlock();
	ProcedureBlock(pd, Combine(stop, T_SEMICOLON, -1));
	Expect(T_SEMICOLON, stop);
	SymTab->ExitBlock();
}

void parser_t::ProcedureBlock(procedure_t *p, set_t stop)
{
	set_t stop2 = Union(stop, blockfirst);
	int vars = Code->NewLabel();
	int temps = Code->NewLabel();
	int start = Code->NewLabel();
	int paramsize = 0;
	if (Lex->Token() == T_LEFTPAREN)
	{
		Expect(T_LEFTPAREN, Combine(stop2, T_VAR, T_IDENT,
			T_RIGHTPAREN, T_SEMICOLON, -1));
		p->LastParam(FormalParameters(paramsize,
			Combine(stop2, T_RIGHTPAREN, T_SEMICOLON, -1)));
		Expect(T_RIGHTPAREN, Combine(stop2, T_SEMICOLON, -1));
	}
	Expect(T_SEMICOLON, stop2);
	Code->Emit(OP_DEFADDR, p->Label());
	Code->Emit(OP_PROCEDURE, vars, temps, start);
	Block(vars, temps, start, Debugger ? Debugger->ScopeIdx() : 0, stop);
	Code->Emit(OP_ENDPROC, paramsize);
}

symtab_entry_t *parser_t::FormalParameters(int &size, set_t stop)
{
	set_t stop2 = Combine(stop, T_SEMICOLON, -1);
	symtab_entry_t *lastparam;
	for (;;)
	{
		int sz;
		lastparam = ParameterDefinition(sz, stop2);
		size += sz;
		if (Lex->Token() == T_SEMICOLON)
			Expect(T_SEMICOLON, Combine(stop, T_IDENT, T_VAR, -1));
		else break;
	}
	lastparam->AllocateAddress(VALUEPARAM, 0);
	return lastparam;
}

symtab_entry_t *parser_t::ParameterDefinition(int &size, set_t stop)
{
	symtab_entry_t *last, *type;
	int cnt = 0;
	if (Lex->Token() == T_VAR)
	{
		Expect(T_VAR, Combine(stop, T_IDENT, -1));
		type = VariableGroup(REFPARAM, last, cnt, stop);
	}
	else type = VariableGroup(VALUEPARAM, last, cnt, stop);
	size = cnt * type->Size();
	return last;
}

void parser_t::Statement(set_t stop)
{
	switch(Lex->Token())
	{
	case T_IDENT:
		// Procedure call or var ref
		symtab_entry_t *se = SymTab->Find(Lex->TokenVal());
		switch (se->Class())
		{
			case VARIABLE:
			case VALUEPARAM:
			case REFPARAM:
				AssignmentStmt(stop);
				break;
			case PROCEDURE:
			case STANDARDPROC:
				ProcedureStmt(stop);
				break;
			default:
				TypeError(se, "procedure, variable or parameter");
				break;
		}
		break;
	case T_IF:
		IfStmt(stop);
		break;
	case T_WHILE:
		WhileStmt(stop);
		break;
	case T_BEGIN:
		CompoundStmt(stop);
		break;
	default: // empty statement
		Synchronise(stop);
		break;
	}
}

void parser_t::AssignmentStmt(set_t stop)
{
	set_t stop2 = Union(stop, exprfirst);
	symtab_entry_t *vtype, *etype;
	vtype = VariableAccess(Combine(stop2, T_ASSIGN, -1));
	Expect(T_ASSIGN, stop2);
	etype = Expression(stop);
	CheckTypes(vtype, etype);
	Code->Emit(OP_ASSIGN, vtype->Size());
	SymTab->Pop(1+vtype->Size());
}

void parser_t::ProcedureStmt(set_t stop)
{
	procedure_t *p = (procedure_t *)SymTab->Find(Lex->TokenVal());
	if (p->Class() == STANDARDPROC)
		StandardProcStmt(stop);
	else
	{
		int plen = 0;
		if (p->LastParam())
		{
			set_t stop2 = Combine(stop, T_RIGHTPAREN, -1);
			Ident(Combine(Union(stop2, exprfirst),
				T_LEFTPAREN, -1));
			Expect(T_LEFTPAREN, Union(stop2, exprfirst));
			plen = ActualParameters(p->LastParam(), stop2);
			Expect(T_RIGHTPAREN, stop);
		}
		else Ident(stop);
		Code->Emit(OP_PROCCALL, SymTab->Level() - p->Level(),
			p->Label());
		SymTab->Push(3);
		SymTab->Pop(3+plen);
	}
}

void parser_t::StandardProcStmt(set_t stop)
{
	set_t stop2 = Combine(stop, T_RIGHTPAREN, -1);
	int id = Ident(Union(stop2, exprfirst));
	Expect(T_LEFTPAREN, Union(stop2, exprfirst));
	symtab_entry_t *argtype;
	if (id == read_proc_idx)
		argtype = VariableAccess(stop2);
	else if (id == write_proc_idx)
		argtype = Expression(stop2);
	CheckTypes(argtype, integer_type);
	Code->Emit( (id == read_proc_idx) ? OP_READ : OP_WRITE);
	Expect(T_RIGHTPAREN, stop);
	SymTab->Pop(1);
}

int parser_t::ActualParameters(symtab_entry_t *last_param, set_t stop)
{
	int len = 0;
	if (last_param->Next())
	{
		len = ActualParameters(last_param->Next(),
			Combine(Union(stop, exprfirst), T_COMMA, -1));
		Expect(T_COMMA, Union(stop, exprfirst));
	}
	symtab_entry_t *ptype;
	if (last_param->Class() == VALUEPARAM)
	{
		ptype = Expression(stop);
		CheckTypes(ptype, ((valueparam_t *)last_param)->Type());
		len += ptype->Size();
	}
	else
	{
		ptype = VariableAccess(stop);
		CheckTypes(ptype, ((refparam_t *)last_param)->Type());
		len++;
	}
	return len;
}

void parser_t::IfStmt(set_t stop)
{
	Expect(T_IF, Combine(Union(stop,
		Union(exprfirst, stmtfirst)), T_THEN, T_ELSE, -1));
	symtab_entry_t *etype = Expression(Combine(Union(stop, stmtfirst),
		T_THEN, T_ELSE, -1));
	if (etype != boolean_type)
		TypeError(etype, "boolean expression");
	int after = Code->NewLabel();
	Code->Emit(OP_DO, after);
	SymTab->Pop(1);
	Expect(T_THEN, Combine(Union(stop, stmtfirst), T_ELSE, -1));
	Statement(Combine(stop, T_ELSE, -1));
	int end = Code->NewLabel();
	Code->Emit(OP_GOTO, end);
	Code->Emit(OP_DEFADDR, after);
	if (Lex->Token() == T_ELSE)
	{
		Expect(T_ELSE, Union(stop, stmtfirst));
		Statement(stop);
	}
	Code->Emit(OP_DEFADDR, end);
}

void parser_t::WhileStmt(set_t stop)
{
	Expect(T_WHILE, Combine(Union(stop,
		Union(exprfirst, stmtfirst)), T_DO, -1));
	int start = Code->NewLabel();
	Code->Emit(OP_DEFADDR, start);
	symtab_entry_t *etype = Expression(Combine(Union(stop, stmtfirst),
		T_DO, -1));
	if (etype != boolean_type)
		TypeError(etype, "boolean expression");
	Expect(T_DO, Union(stop, stmtfirst));
	SymTab->Pop(1);
	int end = Code->NewLabel();
	Code->Emit(OP_DO, end);
	Statement(stop);
	Code->Emit(OP_GOTO, start);
	Code->Emit(OP_DEFADDR, end);
}

void parser_t::CompoundStmt(set_t stop)
{
	Expect(T_BEGIN, Combine(Union(stop, stmtfirst),
		T_SEMICOLON, T_END, -1));
	set_t stop2 = Combine(stop, T_SEMICOLON, T_END, -1);
	for (;;)
	{
		Statement(stop2);
		if (Lex->Token() == T_SEMICOLON)
			Expect(T_SEMICOLON, Union(stop2, stmtfirst));
		else break;
	}
	Expect(T_END, stop);
}

symtab_entry_t *parser_t::Expression(set_t stop)
{
	symtab_entry_t *ltype = SimpleExpr(Union(stop, relops));
	token_t op = Lex->Token();
	if (In(relops, op))
	{
		Lex->NextToken();
		symtab_entry_t *rtype = SimpleExpr(stop);
		ltype = CheckTypes(ltype, rtype);
		if (ltype != integer_type)
		{
			if (op != T_EQUALS && op != T_NOTEQUAL)
				TypeError(ltype, "integer");
		}
		switch(op)
		{
		case T_EQUALS:
			Code->Emit(OP_EQUAL);	break;
		case T_NOTEQUAL:
			Code->Emit(OP_NOTEQUAL);break;
		case T_GREATER:
			Code->Emit(OP_GREATER);	break;
		case T_GREATEREQU:
			Code->Emit(OP_NOTLESS);	break;
		case T_LESS:
			Code->Emit(OP_LESS);	break;
		case T_LESSEQU:
			Code->Emit(OP_NOTGREATER); break;
		}
		SymTab->Pop(1);
		ltype = boolean_type;
	}
	Synchronise(stop);
	return ltype;
}

symtab_entry_t *parser_t::SimpleExpr(set_t stop)
{
	int neg = -1;
	if (Lex->Token() == T_PLUS)
	{
		neg = 0;
		Lex->NextToken();
	}
	else if (Lex->Token() == T_MINUS)
	{
		neg = 1;
		Lex->NextToken();
	}
	symtab_entry_t *ltype;
	if (neg>=0)
	{
		ltype = Term(stop);
		if (neg) Code->Emit(OP_MINUS);
	}
	else
	{
		set_t stop2 = Union(stop, addops);
		ltype = Term(stop2);
		while (In(addops, Lex->Token()))
		{
			token_t op = Lex->Token();
			Lex->NextToken();
			symtab_entry_t *rtype = Term(stop2);
			if (ltype == integer_type)
			{
				ltype = CheckTypes(ltype, rtype);
				switch (op)
				{
					case T_PLUS:
						Code->Emit(OP_ADD);	break;
					case T_MINUS:
						Code->Emit(OP_SUBTRACT);break;
					case T_OR:
						TypeError(ltype, "boolean");
				}
				SymTab->Pop(1);
			}
			else if (ltype == boolean_type)
			{
				ltype = CheckTypes(ltype, rtype);
				if (neg >= 0 || op != T_OR)
					TypeError(ltype, "integer");
				else Code->Emit(OP_OR);
				SymTab->Pop(1);
			}
			else TypeError(ltype, "integer or boolean");
		}
	}
	Synchronise(stop);
	return ltype;
}

symtab_entry_t *parser_t::Term(set_t stop)
{
	set_t stop2 = Union(stop, mulops);
	symtab_entry_t *ltype = Factor(stop2);
	while (In(mulops, Lex->Token()))
	{
		token_t op = Lex->Token();
		Lex->NextToken();
		symtab_entry_t *rtype = Factor(stop2);
		if (ltype == integer_type)
		{
			ltype = CheckTypes(ltype, rtype);
			switch(op)
			{
				case T_ASTERISK:
					Code->Emit(OP_MULTIPLY);
					break;
				case T_DIV:
					Code->Emit(OP_DIV);
					break;
				case T_MOD:
					Code->Emit(OP_MODULO);
					break;
				case OP_AND:
					TypeError(ltype, "boolean");
					break;
			}
			SymTab->Pop(1);
		}
		else if (ltype == boolean_type)
		{
			ltype = CheckTypes(ltype, rtype);
			if (op != T_AND)
				TypeError(ltype, "integer");
			Code->Emit(OP_AND);
			SymTab->Pop(1);
		}
		else TypeError(ltype, "integer or boolean");
	}
	Synchronise(stop);
	return ltype;
}

symtab_entry_t *parser_t::Factor(set_t stop)
{
	symtab_entry_t *etype;
	switch(Lex->Token())
	{
		case T_NOT:
			Expect(T_NOT, Combine(stop, T_LEFTPAREN, T_IDENT,
				T_NOT, T_INTEGER, -1));
			etype = Factor(stop);
			if (etype != boolean_type)
				TypeError(etype, "boolean");
			Code->Emit(OP_NOT);
			break;
		case T_LEFTPAREN:
			Expect(T_LEFTPAREN, Combine(Union(stop, exprfirst),
				T_RIGHTPAREN, -1));
			etype = Expression(Combine(stop, T_RIGHTPAREN, -1));
			Expect(T_RIGHTPAREN, stop);
			break;
		case T_INTEGER:
		{
			int v;
			Constant(v, etype, stop);
			Code->Emit(OP_CONSTANT, v);
			SymTab->Push(1);
			break;
		}
		case T_IDENT:
		{
			symtab_entry_t *se = SymTab->Find(Lex->TokenVal());
			switch(se->Class())
			{
				case CONSTANT:
				{
					int v;
					Constant(v, etype, stop);
					Code->Emit(OP_CONSTANT, v);
					SymTab->Push(1);
					break;
				}
				case VALUEPARAM:
				case REFPARAM:
				case VARIABLE:
					etype = VariableAccess(stop);
					Code->Emit(OP_VALUE, etype->Size());
					SymTab->Push(etype->Size() - 1);
					break;
				default:
					TypeError(se, "variable, parameter or constant");
					etype = unknown_type;
					break;
			}
			break;
		}
		default:
			ErrorHandler->Error(ERR_EXPRSYNTAX);
			etype = unknown_type;
			break;
	}
	return etype;
}

symtab_entry_t *parser_t::VariableAccess(set_t stop)
{
	set_t stop2 = Union(stop, selectors);
	symtab_entry_t *se = SymTab->Find(Ident(stop2)), *type;
	switch (se->Class())
	{
		case VARIABLE:
		{
			variable_t *v = (variable_t *)se;
			type = v->Type();
			Code->Emit(OP_VARIABLE,
				SymTab->Level() - v->Level(), v->Displ());
			SymTab->Push(1);
			break;
		}
		case VALUEPARAM:
		{
			valueparam_t *vp = (valueparam_t *)se;
			type = vp->Type();
			Code->Emit(OP_VARIABLE,
				SymTab->Level() - vp->Level(), vp->Displ());
			SymTab->Push(1);
			break;
		}
		case REFPARAM:
		{
			refparam_t *rp = (refparam_t *)se;
			type = rp->Type();
			Code->Emit(OP_REFPARAM,
				SymTab->Level() - rp->Level(), rp->Displ());
			SymTab->Push(1);
			break;
		}
		default:
			TypeError(se, "variable or parameter");
			type = unknown_type;
			break;
	}
	while (In(selectors, Lex->Token()))
		type = Selector(type, stop2);
	return type;
}

symtab_entry_t *parser_t::Selector(symtab_entry_t *type, set_t stop)
{
	switch(Lex->Token())
	{
		case T_PERIOD:
			return FieldSelector((recordtype_t *)type, stop);
		case T_LEFTBRACKET:
			return IndexSelector((arraytype_t *)type, stop);
	}
	assert(0); return NULL;
}

symtab_entry_t *parser_t::IndexSelector(arraytype_t *array_type, set_t stop)
{
	Expect(T_LEFTBRACKET, Combine(Union(stop, exprfirst),
		T_RIGHTBRACKET, -1));
	symtab_entry_t *index_type = Expression(Combine(stop, T_RIGHTBRACKET, -1));
	Expect(T_RIGHTBRACKET, stop);
	if (array_type->Class() != ARRAYTYPE)
	{
		TypeError(array_type, "array");
		Synchronise(stop);
		return unknown_type;
	}
	(void)CheckTypes(index_type, array_type->IndexType());
	Code->Emit(OP_INDEX, array_type->Lower(), array_type->Upper(),
		array_type->ElementType()->Size());
	SymTab->Pop(1);
	return array_type->ElementType();
}

symtab_entry_t *parser_t::FieldSelector(recordtype_t *record_type, set_t stop)
{
	Expect(T_PERIOD, Combine(stop, T_IDENT, -1));
	int id = Ident(stop);
	if (record_type->Class() != RECORDTYPE)
	{
		TypeError(record_type, "record");
		return unknown_type;
	}
	fieldtype_t *rtn = (fieldtype_t *)SymTab->FindField(record_type, id);
	if (rtn == NULL)
	{
		ErrorHandler->Error(ERR_NOFIELD, NameStore->Name(id));
		Synchronise(stop);
		return unknown_type;
	}
	Code->Emit(OP_FIELD, rtn->Displ());
	return rtn->Type();
}

void parser_t::Constant(int &val, symtab_entry_t* &type, set_t stop)
{
	if (Lex->Token() == T_INTEGER)
	{
		val = Lex->TokenVal();
		type = integer_type;
		Expect(T_INTEGER, stop);
	}
	else if (Lex->Token() == T_IDENT)
	{
		symtab_entry_t *e = SymTab->Find(Ident(stop));
		if (e->Class() == CONSTANT)
		{
			val = ((constant_t *)e)->Value();
			type = ((constant_t *)e)->Type();
		}
		else
		{
			TypeError(e, "constant");
			val = 0;
			type = unknown_type;
		}
	}
	else
	{
		ErrorHandler->Error(ERR_CONSTANT);
	 	val = 0;
	 	type = unknown_type;
		Synchronise(stop);
	}
}


int parser_t::Ident(set_t stop)
{
	int rtn;
	if (Lex->Token() == T_IDENT)
	{
		rtn = Lex->TokenVal();
		Lex->NextToken();
		Synchronise(stop);
	}
	else
	{
		rtn = no_name;
		Expect(T_IDENT, stop);
	}
	return rtn;
}

