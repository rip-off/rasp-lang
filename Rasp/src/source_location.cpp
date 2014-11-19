#include "source_location.h"

#include <iostream>

SourceLocation::SourceLocation(const std::string &filename, unsigned line)
:
	line_(line),
	filename_(filename)
{
}

unsigned SourceLocation::line() const
{
	return line_;
}

std::ostream &operator<<(std::ostream &out, const SourceLocation &sourceLocation)
{
	return out << sourceLocation.filename_ << " at line " << sourceLocation.line_;
}

