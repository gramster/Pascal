//-----------------------------------------------------------------------
// lex.h - Header file for lexical analyser class of Pascal- compiler
//
// Written by G. Wheeler for PLT Course, 1994
//-----------------------------------------------------------------------

#ifndef __LEX_H
#define __LEX_H

#include <stdio.h>

#define MAX_TOKEN_LENGTH		80

// Token types returned by lexical analyser

typedef enum
{
	// reserved words

	T_AND,			T_ARRAY,		T_BEGIN,		T_CONST,
	T_DIV,			T_DO,			T_ELSE,			T_END,
	T_IF,			T_MOD,			T_NOT,			T_OF,
	T_OR,			T_PROCEDURE,	T_PROGRAM,		T_RECORD,
	T_THEN,			T_TYPE,			T_VAR,			T_WHILE,

	// Identifiers and literals

	T_IDENT,		T_INTEGER,

	// Punctuation

	T_PLUS,			T_MINUS,		T_ASTERISK,		T_LEFTPAREN,
	T_RIGHTPAREN,	T_LEFTBRACKET,	T_RIGHTBRACKET,	T_SEMICOLON,
	T_EQUALS,		T_GREATEREQU,	T_GREATER,		T_LESSEQU,
	T_NOTEQUAL,		T_LESS,			T_ASSIGN,		T_COLON,
	T_DOUBLEDOT,	T_PERIOD,		T_COMMA,

	// EOF and undefined

	T_EOF, T_UNDEFINED

} token_t;

class tokeniser_t
{
private:
	FILE*	source_fp;
	int		ch;
	token_t	token_type;
	char	token_name[MAX_TOKEN_LENGTH+1];
	int		token_value;
	int		line_number;
	token_t	LookupName();
	void	NextChar();
	void	StripComment();
	void	ScanInteger();
	void	ScanName();
public:
	token_t	NextToken();
	token_t	Token()
	{
		return token_type;
	}
	int	TokenVal()
	{
		return token_value;
	}
	tokeniser_t(char *source_file_name);
	int Line()
	{
		return line_number;
	}
	const char *TokenName()
	{
		return token_name;
	}
	const char *TokenID(token_t t);
};

extern tokeniser_t *Lex;

#endif

