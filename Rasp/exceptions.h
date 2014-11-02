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
