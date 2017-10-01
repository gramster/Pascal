#include <stdio.h>
#include "pmachine.h"
#include "debugger.h"

#include <assert.h>
#include "debugger.h"

debugger_t *Debugger;

// We don't link in the debugger class; instead, we define the
// class here to be a set of do-nothing methods. Use the pm -d
// command for symbolic debugging, as prun has no access to the
// symbol table at present.

debugger_t::debugger_t()			{ }
pword_t debugger_t::ScopeIdx()			{ return 0;	}
void debugger_t::EnterScope(pword_t sidx)	{ }
void debugger_t::ExitScope()			{ }
void debugger_t::SourceLine(pword_t ln)		{ }
void debugger_t::Paginate(char *source)		{ }
void debugger_t::ShowSource(int ln, int cnt)	{ }
int debugger_t::GetCommand()			{ }

static void useage()
{
	fprintf(stderr, "Useage: prun\n");
	exit(-1);
}

main(int argc, char *argv[])
{
	Debugger = NULL;
	if (argc!=1) useage();
	pmachine_t *tim = new pmachine_t;
	tim->LoadAndGo("pm.cod");
	delete tim;
	return 0;
}

