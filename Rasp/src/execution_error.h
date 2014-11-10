#ifndef EXECUTION_ERROR_H
#define EXECUTION_ERROR_H

#include "exceptions.h"

class ExecutionError : public RaspError
{
public:
	ExecutionError(unsigned line, const std::string &message)
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

#endif
