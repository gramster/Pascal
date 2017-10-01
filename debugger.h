//---------------------------------------------------
// Basic symbolic debugging support

#ifndef __DEBUG_H
#define __DEBUG_H

#include "symtab.h"

#define MAX_DEBUG_SCOPES		128
#define MAX_LINES				256

typedef enum
{
	STEP, RUN, SET, CLEAR, QUIT, SHOW
} debug_cmd_t;

#define BRK_SET_SIZE		((MAX_LINES+31)/32)

class debugger_t
{
private:
	FILE			*sourcefp;		// source file pointer
	symtab_entry_t	*scopestore[MAX_DEBUG_SCOPES];	// used to recreate symtab
	int				storeidx;				// number of entries in scopestore
	long			offsets[MAX_LINES];		// source file pagination table
	unsigned long	breaks[BRK_SET_SIZE];	// line breakpoint bitmask
	int				numlines;		// number of source lines
	int				linenow;		// current source line
	int				stepcnt;		// number of lines to execute
	debug_cmd_t		command;		// last execute command
	void Paginate();
public:
	debugger_t(char *source);
	~debugger_t();
	pword_t ScopeIdx();
	void EnterScope(pword_t sidx);
	void ExitScope();
	int  SourceLine(pword_t ln);
  int  DoCommands();
	void ShowSource(int ln = -1, int cnt = 1);
	int GetCommand();
};

extern debugger_t *Debugger;

#endif




