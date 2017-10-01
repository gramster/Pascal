// Pascal- p-machine by G. Wheeler

#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#ifdef DEBUG
#include <conio.h>
int debug = 0;
#endif
#include "pmachine.h"
#include "debugger.h"

pmachine_t *Interp;

static char *err_msg_tbl[] =
{
	"Cannot open code file %s",
	"Division by zero",
	"Stack overflow",
	"%s"
};

void pmachine_t::RunTimeError(RT_error_t err, char *arg)
{
	fprintf(stderr, "Runtime error! ");
	fprintf(stderr, err_msg_tbl[err], arg);
	fprintf(stderr, "\nAborted\n");
	exit(-1);
}

pword_t pmachine_t::Chain(pword_t level) // follow activation chain
{
	pword_t v = b;
	assert(level>=0);
	while (level--) v = Abs(v);
	return v;
}

int pmachine_t::ExecOp()
{
	pword_t tmp;
#ifdef DEBUG
	if (debug>1)
	{
		pword_t tmps = code_top;
		while (tmps<=s)
		{
			if (sizeof(pword_t) == sizeof(long))
				printf("[%5ld:%8lX] %s\n",tmps,store[tmps]);
			else
				printf("[%5d:%6d] %s\n",tmps,store[tmps]);
			tmps++;
		}
	}
	if (debug)
	{
		pword_t tmpp = p;
		if (sizeof(pword_t) == sizeof(long))
			printf("[%5ld:%8lX] %s\n",s,store[s],DisassembleOp(store,tmpp));
		else
			printf("[%5d:%4X] %s\n",s,store[s],DisassembleOp(store,tmpp));
#if __MSDOS__
		//if (getch() == 27) return 0;
#else
		do
		{
			int ch = getchar();
			if (ch==27) return 0;
		} while (ch != '\n');
#endif
	}
#endif
	p_code_op_t op = (p_code_op_t)store[p++];
	pword_t *args = store+p;
	p += op_argcnt_tbl[op];
	switch (op)
	{
	// Basic arithmetic ops (all OK)

	case OP_ADD:	Push(Pop()+Pop());		break;
	case OP_AND:	Push(Pop()&Pop());		break;
	case OP_ASSIGN:
	{
		pword_t  val	= Drop(args[0]) + 1;	// get start of value
		pword_t  addr	= Pop();		// pop address
		memcpy(store+addr, store+val, args[0]*sizeof(pword_t));
		break;
	}
	case OP_CONSTANT:	Push(args[0]);		break;
	case OP_DEFADDR:
	case OP_DEFARG:		assert(0);
	case OP_DIV:	
		if (TOS()==0) RunTimeError(ERR_ZERODIV);
		else { Pop(&tmp); Push(Pop()/tmp); }
		break;
	case OP_DO:	if (!Pop()) p += args[0];		break;
	case OP_ENDPROC:
		s = b - args[0]; // pop vars and activation record
		p = store[b+2]; // restore p and b from AR
		b = store[b+1];
		break;
	case OP_ENDPROG:
		return 0;
	case OP_ENDSCOPE:
		if (Debugger)
			Debugger->ExitScope();
		break;
	case OP_EQUAL:		Push( Pop() == Pop() );		break;
	case OP_FIELD:		store[s] += args[0];		break;
	case OP_GLOBALCALL:
		Push(Chain(1));
		Push(b);
		Push(p);
		b = s - 2;
		p += args[0];
		break;
	case OP_GLOBALVALUE:	Push(Abs(Abs(b) + args[0]));	break;
	case OP_GLOBALVAR:	Push(Abs(b) + args[0]);		break;
	case OP_GOTO:		p += args[0];			break;
	case OP_GREATER:	Pop(&tmp); Push(Pop()>tmp);	break;
	case OP_INDEX:
	{
		pword_t ix = Pop();
		if (ix < args[0] || ix > args[1])
		{
			char buf[80];
			printf(buf, "Value %d not in range [%d,%d]",ix,
				args[0], args[1]);
			RunTimeError(ERR_CUSTOM,buf);
		}
		else store[s] += (ix-args[0]) * args[2];
		break;
	}
	case OP_LESS:		Pop(&tmp); Push(Pop()<tmp);	break;
	case OP_LOCALVALUE:	Push(Abs(b + args[0]));		break;
	case OP_LOCALVAR:	Push(b+args[0]);		break;
	case OP_MINUS:		Push(-Pop());			break;
	case OP_MODULO:		Pop(&tmp); Push(Pop()%tmp);	break;
	case OP_MULTIPLY:	Push( Pop() * Pop() );		break;
	case OP_NEWLINE:
		if (Debugger)
			if (Debugger->SourceLine(args[0]))
				return 0; // user requested quit
		break;
	case OP_NOT:		Push( !Pop() );			break;
	case OP_NOTEQUAL:	Push( Pop() != Pop() );		break;
	case OP_NOTGREATER:	Pop(&tmp); Push(Pop()<=tmp);	break;
	case OP_NOTLESS:	Pop(&tmp); Push(Pop()>=tmp);	break;
	case OP_OR:		Push(Pop() | Pop() );		break;
	case OP_PROCCALL:
		Push(Chain(args[0]));
		Push(b);
		Push(p);
		b = s - 2;
		p += args[1];
		break;
	case OP_PROCEDURE:
		s += args[0]; // reserve space for local vars
		if ((s + args[1]) >= STORE_SIZE)
			RunTimeError(ERR_STACK);
		p += args[2];
		break;
	case OP_PROGRAM:
		b = code_top;
		s = b + 2 + args[0];
		if ((s + args[1]) >= STORE_SIZE)
			RunTimeError(ERR_STACK);
		p += args[2];
		break;
	case OP_READ:
		Pop(&tmp);
		if (sizeof(pword_t)==sizeof(long))
			while (scanf("%ld", &store[tmp])!=1);
		else
			while (scanf("%d", &store[tmp])!=1);
		break;
	case OP_REFPARAM:	// Push address of bound variable
		Push(Abs(Chain(args[0]) + args[1]));
		break;
	case OP_SCOPE:
		if (Debugger) Debugger->EnterScope(args[0]);
		break;
	case OP_SIMPLEASSIGN:	Pop(&tmp); Abs(Pop(), tmp);	break;
	case OP_SIMPLEVALUE:	Push(Abs(Pop()));		break;
	case OP_SUBTRACT:	Pop(&tmp); Push(Pop()-tmp);	break;
	case OP_VALUE:
	{
		pword_t length 	= args[0];
		pword_t addr	= Pop();
		while (length--)
			Push(Abs(addr++));
		break;
	}
	case OP_VARIABLE:	// Push address of variable
		Push(Chain(args[0]) + args[1]);
		break;
	case OP_WRITE:
		printf((sizeof(pword_t)==sizeof(long))?"%ld\n":"%d\n", Pop());
		break;
	}
	return 1;
}

void pmachine_t::LoadAndGo(char *codefile)
{
#if __MSDOS__
	FILE *fp = fopen(codefile, "rb");
#else
	FILE *fp = fopen(codefile, "r");
#endif
	if (fp)
		code_top = fread(store, sizeof(pword_t), STORE_SIZE, fp);
	else RunTimeError(ERR_INPUT, codefile);
	fclose(fp);
	p = 0;
	s = code_top;
	while (ExecOp());
}


