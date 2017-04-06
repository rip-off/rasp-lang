#ifndef EXECUTION_ERROR_H
#define EXECUTION_ERROR_H

#include "exceptions.h"
#include "source_location.h"

class ExecutionError : public RaspError
{
public:
	ExecutionError(const SourceLocation &sourceLocation, const std::string &message)
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

#endif
