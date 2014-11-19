#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "exceptions.h"

class LexError : public RaspError
{
public:
	LexError(const SourceLocation &sourceLocation, const std::string &message)
	: 
		RaspError(message),
		sourceLocation_(sourceLocation)
	{
	}

	SourceLocation sourceLocation() const
	{
		return sourceLocation_;
	}

private:
	SourceLocation sourceLocation_;
};

Token lex(const std::string &filename, const std::string &source);

#endif
