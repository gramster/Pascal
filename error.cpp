//-----------------------------------------------------------------------
// error.cpp - error handler class of Pascal- compiler
//
// Written by G. Wheeler for PLT Course, 1994
//-----------------------------------------------------------------------

#include <stdio.h>
#include <ctype.h>
#include "error.h"
#include "lex.h"

static int error_counts[3] =
{
	0, 0, 0
};

static char *error_class_names[3] =
{
	"Warning",
	"Error",
	"Fatal Error"
};

static char *errorMessage[] =
{
	"End of file reached while processing comment",
	"Integer is out of alowed range",
	"Illegal character %s",
	"Cannot open %s for input",
	"Maximum allowed number of unique identifiers exceeded",
	"Token is too long; truncated to %s",
	"Trailing garbage in file after end of program",
	"%s expected",
	"ARRAY or RECORD expected",
	"Undefined identifier %s",
	"Redefinition of identifier %s",
	"Constant expected",
	"Object of type %s expected",
	"Invalid range (bottom exceeds top)",
	"Syntax error",
	"Expression syntax error",
	"No field named %s in record",
	"Incompatible types",
	"Cannot open output code file `%s'"
};

int error_handler_t::Warnings()
{
	return error_counts[WARN];
}

int error_handler_t::Errors()
{
	return error_counts[ERROR];
}

void error_handler_t::_Error(int severity, const error_t err, const char *arg)
{
	char place[128];
	char msg[128];
	// we have to check for ERR_NOSOURCE as this error occurs in
	// the constructor for Lex, so we can't check TokenName yet
	if (err == ERR_NOSOURCE) place[0] = 0;
	else if (isprint(Lex->TokenName()[0]))
		sprintf(place, "line %d near <%s>", Lex->Line(), Lex->TokenName());
	else
		sprintf(place, "line %d", Lex->Line());
	sprintf(msg, errorMessage[(int)err], arg);
	fprintf(stderr,"[%s %d] %s %s\n",
		error_class_names[severity],
		++error_counts[severity],
		place, msg);
	if (severity == FATAL) exit(-1);
}

