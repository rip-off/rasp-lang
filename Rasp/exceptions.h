#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

class RaspError : public std::runtime_error
{
public:
	RaspError(const std::string &message)
		: std::runtime_error(message)
	{
	}
};

class LexError : public RaspError
{
public:
	LexError(const std::string &message)
		: RaspError(message)
	{
	}
};

class ParseError : public RaspError
{
public:
	ParseError(const std::string &message)
		: RaspError(message)
	{
	}
};

class ExecutionError : public RaspError
{
public:
	ExecutionError(const std::string &message)
		: RaspError(message)
	{
	}
};

#endif
