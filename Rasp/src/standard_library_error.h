#ifndef STANDARD_LIBRARY_ERROR_H
#define STANDARD_LIBRARY_ERROR_H

#include "execution_error.h"

class ExternalFunctionError : public ExecutionError
{
public:
	ExternalFunctionError(const std::string &function, const SourceLocation &sourceLocation, const std::string &message)
	:
		ExecutionError(sourceLocation, message + " in external function '" + function + "'")
	{
	}
};


#endif

