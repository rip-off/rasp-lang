#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "exceptions.h"

class LexError : public RaspError
{
public:
	LexError(unsigned line, const std::string &message)
	: 
		RaspError(message),
		line_(line)
	{
	}

	unsigned line() const
	{
		return line_;
	}

private:
	unsigned line_;
};

Token lex(const std::string &source);

#endif
