#include "exceptions.h"

#include <iostream>

RaspError::~RaspError()
{
}

const RaspError::StackTrace &RaspError::stacktrace() const
{
	return stacktrace_;
}

void RaspError::buildStackTrace(const std::string &entry)
{
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
			out << trace[i] << '\n';
		}
	}
}

