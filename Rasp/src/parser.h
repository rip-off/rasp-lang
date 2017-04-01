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
	ParseError(const SourceLocation &sourceLocation, const std::string &message)
	: 
		RaspError(message),
		sourceLocation_(sourceLocation)
	{
	}

	const SourceLocation &sourceLocation() const
	{
		return sourceLocation_;
	}

private:
	SourceLocation sourceLocation_;
};

InstructionList parse(const Token &tree, Declarations &declarations, const Settings &settings);

#endif
