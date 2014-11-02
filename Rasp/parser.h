#ifndef PARSER_H
#define PARSER_H

#include "bindings.h"
#include "exceptions.h"
#include "instruction.h"

class Token;
class Settings;

class ParseError : public RaspError
{
public:
	ParseError(unsigned line, const std::string &message)
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

InstructionList parse(const Token &tree, const Bindings &bindings, const Settings &settings);

#endif
