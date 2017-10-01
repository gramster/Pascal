#ifndef __PCODE_H
#define __PCODE_H

// Size of the p-machine memory store

#define STORE_SIZE	0x4000

// The word size of the p-machine

typedef short	pword_t;

// The p-code operations

typedef enum
{
	OP_ADD,			OP_AND,			OP_ASSIGN,		OP_CONSTANT,	OP_DEFADDR,
	OP_DEFARG,		OP_DIV,			OP_DO,			OP_ENDPROC,		OP_ENDPROG,
	OP_ENDSCOPE,	OP_EQUAL,		OP_FIELD,		OP_GLOBALCALL,	OP_GLOBALVALUE,
	OP_GLOBALVAR,	OP_GOTO,		OP_GREATER,		OP_INDEX,		OP_LESS,
	OP_LOCALVALUE,	OP_LOCALVAR,	OP_MINUS,		OP_MODULO,		OP_MULTIPLY,
	OP_NEWLINE,		OP_NOT,			OP_NOTEQUAL,	OP_NOTGREATER,	OP_NOTLESS,
	OP_OR,			OP_PROCCALL,	OP_PROCEDURE,	OP_PROGRAM,	  	OP_READ,
	OP_REFPARAM,	OP_SCOPE,		OP_SIMPLEASSIGN,OP_SIMPLEVALUE,	OP_SUBTRACT,
	OP_VALUE,		OP_VARIABLE,	OP_WRITE,
	NUM_OPS
} p_code_op_t;

extern short op_argcnt_tbl[(int)NUM_OPS+1];
extern char *op_name_tbl[(int)NUM_OPS+1];

extern char *DisassembleOp(pword_t *code, pword_t &p);

#endif

