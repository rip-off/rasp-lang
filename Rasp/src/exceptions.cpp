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
	// TODO:
	// stacktrace_.push_back(entry);
	std::cerr << "\t" << entry << '\n';
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

