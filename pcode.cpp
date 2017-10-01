#include <stdlib.h> // for NULL
#include <stdio.h> // for sprintf
#include <string.h> // for strlen

#include "pcode.h"

short op_argcnt_tbl[(int)NUM_OPS+1] = 
{
/* Add              */ 0,
/* And              */ 0,
/* Assign           */ 1,
/* Constant         */ 1,
/* DefAddr          */ 1,
/* DefArg           */ 2,
/* Div              */ 0,
/* Do               */ 1,
/* EndProc	    */ 1,
/* EndProg          */ 0,
/* EndScope         */ 0,
/* Equal            */ 0,
/* Field            */ 1,
/* GlobalCall	    */ 1,
/* GlobalValue	    */ 1,
/* GlobalVar	    */ 1,
/* Goto             */ 1,
/* Greater          */ 0,
/* Index            */ 3,
/* Less             */ 0,
/* LocalValue       */ 1,
/* LocalVar         */ 1,
/* Minus            */ 0,
/* Modulo           */ 0,
/* Multiply         */ 0,
/* NewLine          */ 1,
/* Not              */ 0,
/* NotEqual         */ 0,
/* NotGreater       */ 0,
/* NotLess          */ 0,
/* Or               */ 0,
/* ProcCall         */ 2,
/* Procedure        */ 3,
/* Program          */ 3,
/* Read		    */ 0,
/* RefParam	    */ 2,
/* Scope	    */ 1,
/* SimpleAssign	    */ 0,
/* SimpleValue	    */ 0,
/* Subtract         */ 0,
/* Value            */ 1,
/* Variable         */ 2,
/* Write            */ 0,
/* Consistency check*/ 666
};

char *op_name_tbl[(int)NUM_OPS+1]=
{
	"Add",
	"And",
	"Assign",
	"Constant",
	"DefAddr",
	"DefArg",
	"Div",
	"Do",
	"EndProc",
	"EndProg",
	"EndScope",
	"Equal",
	"Field",
	"GlobalCall",
	"GlobalValue",
	"GlobalVar",
	"Goto",
	"Greater",
	"Index",
	"Less",
	"LocalValue",
	"LocalVar",
	"Minus",
	"Modulo",
	"Multiply",
	"NewLine",
	"Not",
	"NotEqual",
	"NotGreater",
	"NotLess",
	"Or",
	"ProcCall",
	"Procedure",
	"Program",
	"Read",
	"RefParam",
	"Scope",
	"SimpleAssign",
	"SimpleValue",
	"Subtract",
	"Value",
	"Variable",
	"Write",
	NULL
};

char *DisassembleOp(pword_t *code, pword_t &p)
{
	static char buf[80];
	if (p<0 || p>=STORE_SIZE || code[p] < OP_ADD || code[p] > OP_WRITE)
	{
		sprintf(buf,"Invalid op at address %d!", p);
		return buf;
	}
	int cnt = op_argcnt_tbl[code[p]];
	sprintf(buf,"%d\t%s", p, op_name_tbl[code[p]]);
	char *fmt = (sizeof(pword_t)==sizeof(int)) ? " %X%c" : " %lX%c";
	if (cnt > 3)
	{
		strcat(buf,"ERROR!");
		return buf;
	}
	for (int i = 1; i<=cnt; i++)
		sprintf(buf+strlen(buf), fmt, code[p+i], (i<cnt) ? ',' : '\0');
	p += cnt+1;
	return buf;
}
