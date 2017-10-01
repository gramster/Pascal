#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if __MSDOS__
#include <conio.h>
#endif
#include <ctype.h>
#include "debugger.h"

debugger_t *Debugger;

void debugger_t::Paginate()
{
	char buff[132];
	while (!feof(sourcefp))
	{
		offsets[++numlines] = ftell(sourcefp);
		fgets(buff, 132, sourcefp); // assumes linelength < 132!
	}
}

debugger_t::debugger_t(char *source)
{
	storeidx = numlines = 0;
	sourcefp = fopen(source, "r");
	assert(sourcefp);
	Paginate();
	command = STEP;
	stepcnt = 1;
	for (int i = 0; i < BRK_SET_SIZE; i++)
		breaks[i] = 0l;
}

debugger_t::~debugger_t()
{
	fclose(sourcefp);
}

pword_t debugger_t::ScopeIdx()
{
	assert(storeidx < MAX_DEBUG_SCOPES);
	scopestore[storeidx] = SymTab->Last();
	return (pword_t)storeidx++;
}

// EnterScope and ExitScope currently serve no purpose.
// They recreate the scope stack for the currently
// executing procedure and are meant for aiding the
// looking up of values of variables in the debugger,
// which isn't implemented yet.

void debugger_t::EnterScope(pword_t sidx)
{
	SymTab->NewBlock(scopestore[sidx]);
}

void debugger_t::ExitScope()
{
	SymTab->ExitBlock();
}

void debugger_t::ShowSource(int ln, int cnt)
{
	char buff[132];
	if (ln<0) ln = linenow;
	fseek(sourcefp, offsets[ln], 0);
	while (cnt--)
	{
		fgets(buff, 132, sourcefp);
		printf("%-5d %s", ln++, buff);
	}
}

int debugger_t::SourceLine(pword_t ln)
{
	linenow=ln;
	ShowSource();
	return DoCommands();
}

int debugger_t::DoCommands()
{
#if __MSDOS__
	if (!kbhit())
	{
#endif
		if (command == RUN)
		{
			// has this line a breakpoint?
			if ((breaks[linenow/32] & (1l << (linenow%32))) == 0l)
				return 0; // nope
		}
		else
		{
			assert(command == STEP);
			if (--stepcnt > 0) return 0;
		}
#if __MSDOS__
	}
#endif
	char buff[132];
retry:
	fgets(buff, 132, stdin);
	strlwr(buff);
	switch (buff[0])
	{
	case 'h':
		puts("h\t- help\nq\t- quit\nx\t- run\ns[cnt]\t- step");
		puts("b<line>\t- set breakpoint\nb?\t- list breakpoints");
		puts("c<line>\t- clear breakpoint");
		puts("l<line>,<count>\t- list source");
		goto retry;
	case 'q':
		return 1; // quit
	case 'x':
		command = RUN;
		break;
	case 's': // step
		command = STEP;
		if (isdigit(buff[1]))
			stepcnt = atoi(buff+1);
		else stepcnt = 1;
		break;
	case 'b': // set breakpoint
		if (isdigit(buff[1]))
		{
			int ln = atoi(buff+1);
			breaks[ln/32] |= (1l << (ln%32));
		}
		else if (buff[1]=='?') // list breakpoints
		{
			for (int ln=1;ln <= numlines; ln++)
				if ((breaks[ln/32] & (1l << (ln%32))) != 0l)
					printf("%d\n", ln);
		}
		goto retry;
	case 'c': // clear breakpoint
		if (isdigit(buff[1]))
		{
			int ln = atoi(buff+1);
			breaks[ln/32] &= ~(1l << (ln%32));
		}
		goto retry;
	case 'l': // list source
		if (isdigit(buff[1]))
		{
			int rtn, cnt, ln;
			rtn = sscanf(buff+1, "%d,%d", &ln, &cnt);
			if (rtn==0)
				ShowSource();
			else if (rtn==1)
				ShowSource(ln);
			else
				ShowSource(ln, cnt);
		}
		goto retry;
	default:
		goto retry;
	}
	return 0; // don't quit
}
