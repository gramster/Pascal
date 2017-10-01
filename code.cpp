#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "code.h"
#include "symtab.h"
#include "error.h"

class codespace_t *Code;

//----------------------------------------------------------
// Constructor 

codespace_t::codespace_t()
{
	top = 0;
	emitcode = 1;
	nextlabel = 0;
	// do some consistency checks
	assert(op_argcnt_tbl[NUM_OPS] == 666);
	assert(op_name_tbl[NUM_OPS] == NULL);
}

//----------------------------------------------------------
// Code generation

void codespace_t::Emit(p_code_op_t op, ...)
{
	int n, pos=0;
	va_list argptr;
	pword_t arg, copy[MAX_OP_ARGS+1];
	va_start(argptr, op);
	n = op_argcnt_tbl[(int)op];
	assert((top+n+1)<=CODE_SPACE);
	if (emitcode)
		space[top] = (pword_t)op;
	copy[pos++] = (pword_t)op;
	top++;
	while (n--)
	{
		copy[pos] = (pword_t) va_arg(argptr, int);
		if (emitcode)
			space[top] = copy[pos];
		pos++;
		top++;
	}
	va_end(argptr);
}

//--------------------------------------------------------------
// Linking

void codespace_t::NextInstruction()
{
	int i, cnt;
	op = (p_code_op_t)space[in];
	assert(op>=0 && op<NUM_OPS);
	cnt = op_argcnt_tbl[(int)op]+1;
	assert(cnt<MAX_OP_ARGS);
	for (i=0;i<cnt;i++)
		arg[i] = space[in++];
}

pword_t codespace_t::Displ(int argNum)
{
	register pword_t a = arg[argNum];
	if (a == -1 || label_table[a] == -1)
		return (pword_t)-1;
	else return (pword_t)(label_table[a]-nextP);
}

pword_t codespace_t::Value(int argNum)
{
	register pword_t a = arg[argNum];
	if (a == -1) return (pword_t)-1;
	else return label_table[a];
}

void codespace_t::Variable(pword_t level, pword_t disp)
{
	NextInstruction();
	while (op == OP_FIELD)
	{
		disp += arg[1];
		NextInstruction();
	}
	if (level == 0)
	{
		if (op == OP_VALUE && arg[1] == 1)
		{
			Emit(OP_LOCALVALUE, disp);
			NextInstruction();
		}
		else Emit(OP_LOCALVAR, disp);
	}
	else if (level == 1)
	{
		if (op == OP_VALUE && arg[1] == 1)
		{
			Emit(OP_GLOBALVALUE, disp);
			NextInstruction();
		}
		else Emit(OP_GLOBALVAR, disp);
	}
	else Emit(OP_VARIABLE, level, disp);
}

void codespace_t::Process(pword_t pass, pword_t codetop)
{
	emitcode = (pass==1);
	top = in = 0;
	while (in < codetop)
	{
		NextInstruction();
	next: // OP_NEWLINE and OP_VARIABLE use this
		nextP = top + 1 + op_argcnt_tbl[op];
		switch (op)
		{
		// Zero-argument instructions are echoed...

		case OP_ADD:
		case OP_AND:
		case OP_DIV:
		case OP_ENDPROG:
		case OP_ENDSCOPE:
		case OP_EQUAL:
		case OP_GREATER:
		case OP_LESS:
		case OP_MINUS:
		case OP_MODULO:
		case OP_MULTIPLY:
		case OP_NOT:
		case OP_NOTEQUAL:
		case OP_NOTGREATER:
		case OP_NOTLESS:
		case OP_OR:
		case OP_SIMPLEASSIGN:
		case OP_SIMPLEVALUE:
		case OP_SUBTRACT:
			Emit(op);
			break;

		// Single argument ops that require no action:

		case OP_CONSTANT:
		case OP_ENDPROC:
		case OP_GLOBALVALUE:
		case OP_GLOBALVAR:
		case OP_LOCALVALUE:
		case OP_LOCALVAR:
		case OP_NEWLINE:
		case OP_READ:
		case OP_SCOPE:
		case OP_WRITE:
			Emit(op, arg[1]);
			break;

		// Two argument ops that require no action:

		case OP_REFPARAM:
			Emit(op, arg[1], arg[2]);
			break;

		// Three argument ops that require no action:

		case OP_INDEX:
			Emit(op, arg[1], arg[2], arg[3]);
			break;

		// The remaining ops all have link actions:

		case OP_ASSIGN:
			if (arg[1]==1)
				Emit(OP_SIMPLEASSIGN);
			else
				Emit(OP_ASSIGN, arg[1]);
			break;
		case OP_DEFADDR:
			label_table[arg[1]] = top;
			break;
		case OP_DEFARG:
			label_table[arg[1]] = arg[2];
			break;
		case OP_DO:
		case OP_GOTO:
			Emit(op,Displ(1));
			break;
		case OP_FIELD:
			if (arg[1] != 0)
				Emit(OP_FIELD, arg[1]);
			break;
		case OP_PROCCALL:
			if (arg[1] == 1)
				Emit(OP_GLOBALCALL, Displ(2)+1);
			else
				Emit(OP_PROCCALL, arg[1], Displ(2));
			break;
		case OP_PROCEDURE:
		case OP_PROGRAM:
			Emit(op, Value(1), Value(2), Displ(3));
			break;
		case OP_VALUE:
			if (arg[1]==1)
				Emit(OP_SIMPLEVALUE);
			else
				Emit(OP_VALUE,arg[1]);
			break;
		case OP_VARIABLE:
			Variable(arg[1],arg[2]);
			goto next;
		default:
			assert(0); // should deal with each case above
		}
	}
}

int codespace_t::Link()
{
	int codetop = top;
	Process(0,codetop);
	Process(1,codetop);
	return top;
}

void codespace_t::Write(char *codefilename)
{
#if __MSDOS__
	FILE *fp = fopen(codefilename, "wb");
#else
	FILE *fp = fopen(codefilename, "w");
#endif
	if (fp)
	{
		fwrite(space, sizeof(pword_t), top, fp);
		fclose(fp);
	}
	else ErrorHandler->Error(ERR_OUTFILE, codefilename);
}
