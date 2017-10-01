//-----------------------------------------------------------------------
// lex.cpp - lexical analyser class of Pascal- compiler
//
// Written by G. Wheeler for PLT Course, 1994
//-----------------------------------------------------------------------

#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "lex.h"
#include "error.h"
#include "namestor.h"
#include "code.h" // for OP_NEWLINE debugging output

#define MAXINT		(32767)

/*
 * Reserved word lookup
 */

typedef struct
{
	char		*name;
	token_t		token;
} keyword_entry_t;

#define TOKEN(t)	t

static keyword_entry_t keyword_table[] =
{
	{ "and",	T_AND	},
	{ "array",	T_ARRAY	},
	{ "begin",	T_BEGIN	},
	{ "const",	T_CONST	},
	{ "div",	T_DIV	},
	{ "do",		T_DO	},
	{ "else",	T_ELSE	},
	{ "end",	T_END	},
	{ "if",		T_IF	},
	{ "mod",	T_MOD	},
	{ "not",	T_NOT	},
	{ "of",		T_OF	},
	{ "or",		T_OR	},
	{ "procedure",	T_PROCEDURE },
	{ "program",	T_PROGRAM },
	{ "record",	T_RECORD },
	{ "then",	T_THEN	},
	{ "type",	T_TYPE	},
	{ "var",	T_VAR	},
	{ "while",	T_WHILE	}
};
     
//-------------------------------------------------------------------
// Search the reserved word table with a binary search

#define END(v)	(v-1+sizeof(v) / sizeof(v[0]))

token_t tokeniser_t::LookupName()
{
	keyword_entry_t *low = keyword_table,
		       *high = END(keyword_table),
		       *mid;
	int c;
	strlwr(token_name);
	while (low <= high)
	{
		mid = low + (high-low)/2;
		if ((c = strcmp(mid->name, token_name)) == 0)
			return mid->token;
		else if (c < 0)
			low = mid + 1;
		else
			high = mid - 1;
	}
	// It isn't a reserved word, so we enter it into name store

	token_value = (int)(NameStore->Lookup(token_name));
	return (TOKEN(T_IDENT));
}

//-------------------------------------------------------------------

#define EOF	(-1)

void tokeniser_t::NextChar()
{
	if (!feof(source_fp))
		ch = fgetc(source_fp);
	else
		ch = EOF;
}

void tokeniser_t::StripComment()
{
	while (ch != '}' && ch != EOF)
	{
		NextChar();
		// recursive as we allow nested comments
		if (ch == '{') StripComment();
	}
	if (ch == EOF)
		ErrorHandler->Fatal(ERR_COMMENT);
	else NextChar();
}

void tokeniser_t::ScanInteger()
{
	token_value = 0;
	while (isdigit(ch))
	{
		int dval = ch - '0';
		if (token_value > (MAXINT - dval)/10)
			ErrorHandler->Error(ERR_NUMSIZE);
		token_value *= 10;
		token_value += dval;
		NextChar();
	}
}

void tokeniser_t::ScanName()
{
	char *cptr = token_name;
	int len = MAX_TOKEN_LENGTH;
	while (isalnum(ch) || ch=='_')
	{
		if (len>0) *cptr++ = ch;
		NextChar();
		if (len-- == 0)
		{
			*cptr = '\0';
			ErrorHandler->Error(ERR_TOKENLEN, token_name);
		}
	}
	*cptr = '\0';
}

token_t tokeniser_t::NextToken()
{
	token_type = T_UNDEFINED;
	do
	{
		switch (ch)
		{
		case '{':
			StripComment();
			continue;
		case EOF:
			return token_type = T_EOF;
		case '\n':
			line_number++;
			Code->Emit(OP_NEWLINE, line_number);
			break;
		case ' ':
		case '\t':
			break;
		case '+':
			token_type = T_PLUS;
			break;
		case '-':
			token_type = T_MINUS;
			break;
		case '*':
			token_type = T_ASTERISK;
			break;
		case '(':
			token_type = T_LEFTPAREN;
			break;
		case ')':
			token_type = T_RIGHTPAREN;
			break;
		case '[':
			token_type = T_LEFTBRACKET;
			break;
		case ']':
			token_type = T_RIGHTBRACKET;
			break;
		case ',':
			token_type = T_COMMA;
			break;
		case ';':
			token_type = T_SEMICOLON;
			break;
		case '=':
			token_type = T_EQUALS;
			break;
		case '>':
			NextChar();
			if (ch == '=')
				token_type = T_GREATEREQU;
			else
				return token_type = T_GREATER;
			break;
		case '<':
			NextChar();
			if (ch == '=')
				token_type = T_LESSEQU;
			else if (ch == '>')
				token_type = T_NOTEQUAL;
			else
				return token_type = T_LESS;
			break;
		case ':':
			NextChar();
			if (ch == '=')
				token_type = T_ASSIGN;
			else
				return token_type = T_COLON;
			break;
		case '.':
			NextChar();
			if (ch == '.')
				token_type = T_DOUBLEDOT;
			else
				return token_type = T_PERIOD;
			break;
		default:
			if (isdigit(ch))
			{
				ScanInteger();
				return token_type = T_INTEGER;
			}
			else if (isalpha(ch) || ch=='_')
			{
				ScanName();
				return token_type = LookupName();
			}
			else
			{
				token_name[0] = ch;
				token_name[1] = '\0';
				ErrorHandler->Error(ERR_BADCHAR, token_name);
			}
			break;
		}
		NextChar();
	}
	while (token_type == T_UNDEFINED);
	return token_type;
}

tokeniser_t::tokeniser_t(char *source_file_name)
{
	source_fp = fopen(source_file_name, "r");
	token_name[0] = 0;
	line_number = 1;
	Code->Emit(OP_NEWLINE, 1);
	if (source_fp == NULL)
		ErrorHandler->Fatal(ERR_NOSOURCE, source_file_name);
	NextChar();
}

static char *tokenIDs[] =
{
	"AND",		"ARRAY",	"BEGIN",	"CONST",
	"DIV",		"DO",		"ELSE",		"END",
	"IF",		"MOD",		"NOT",		"OF",
	"OR",		"PROCEDURE",	"PROGRAM",	"RECORD",
	"THEN",		"TYPE",		"VAR",		"WHILE",
	"IDENT",	"INTEGER",	"PLUS",		"MINUS",
	"ASTERISK",	"LEFTPAREN",	"RIGHTPAREN",	"LEFTBRACKET",
	"RIGHTBRACKET",	"SEMICOLON",	"EQUALS",	"GREATEREQU",
	"GREATER",	"LESSEQ",	"NOTEQUAL",	"LESS",
	"ASSIGN",	"COLON",	"DOUBLEDOT",	"PERIOD",
	"COMMA",	"EOF",		 "UNDEFINED"
};

const char* tokeniser_t::TokenID(token_t t)
{
	assert(t>=T_AND && t<=T_UNDEFINED);
	return tokenIDs[(int)t];
}

