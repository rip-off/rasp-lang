#include "assert.h"

#include <cstring>
#include "utils.h"

AssertionError::AssertionError(const SourceLocation &sourceLocation, const std::string &message)
:
	message_(message + " at " + str(sourceLocation))
{
}

const char *AssertionError::what() const
{
	return message_.c_str();
}

void assertTrue(const SourceLocation &sourceLocation, bool expression, const std::string &message)
{
	incrementAssertions();
	if(!expression)
	{
		throw AssertionError(sourceLocation, message);
	}
}

void assertEquals(const SourceLocation &sourceLocation, const char *x, const char *y)
{
	incrementAssertions();
	if (std::strcmp(x, y) == 0)
	{
		throw AssertionError(sourceLocation, "'" + str(x) + "' should equal '" + str(y) + "'");
	}
}

namespace
{
	static int assertions = 0;
}

void incrementAssertions()
{
	++assertions;
}

int flushAssertions()
{
	int previous = assertions;
	assertions = 0;
	return previous;
}

