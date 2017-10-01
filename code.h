#ifndef __CODE_H
#define __CODE_H

#include <stdarg.h>

#include "pcode.h"

#define CODE_SPACE	0x4000
#define MAX_OP_ARGS	6

//---------------------------------------------------------------------
// The code store class
//---------------------------------------------------------------------

class codespace_t
{
private:
	pword_t			space[CODE_SPACE];
	pword_t 		arg[MAX_OP_ARGS];
	p_code_op_t		op;
	int				top, in, nextP;
	pword_t			nextlabel;
	pword_t 		label_table[1000];	/* Allocate dynamically of correct size! */
	short         	emitcode;
	void			NextInstruction();
	pword_t			Displ(int argNum);
	pword_t			Value(int argNum);
	void 			Variable(pword_t level, pword_t disp);
	void			Process(pword_t pass, pword_t codetop);
public:
	codespace_t();
	int	NewLabel()
	{
		return nextlabel++;
	}
	void Emit(p_code_op_t, ...);
	int Link();
	void Write(char *codefilename);
};

extern codespace_t *Code;

#endif
