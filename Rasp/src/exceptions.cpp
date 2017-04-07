#include "exceptions.h"

#include <iostream>

RaspError::RaspError(const std::string &message, const SourceLocation &sourceLocation)
		: std::runtime_error(message)
{
	buildStackTrace(message, sourceLocation);
}

RaspError::~RaspError()
{
}

const RaspError::StackTrace &RaspError::stacktrace() const
{
	return stacktrace_;
}

void RaspError::buildStackTrace(const std::string &message, const SourceLocation &sourceLocation)
{
	StackElement entry = { sourceLocation, message };
	stacktrace_.push_back(entry);
}

void printStackTrace(std::ostream &out, const RaspError &e)
{
	const RaspError::StackTrace &trace = e.stacktrace();
	if (trace.empty())
	{
		out << "No stack trace...";
	}
	else
	{
		for(unsigned i = 0 ; i < trace.size() ; ++i)
		{
			out << trace[i].sourceLocation << " " << trace[i].message << '\n';
		}
	}
}

