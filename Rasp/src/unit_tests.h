#ifndef UNIT_TESTS
#define UNIT_TESTS

#include "utils.h"
#include "source_location.h"

#define CURRENT_SOURCE_LOCATION SourceLocation(__FILE__, __LINE__)

class AssertionError
{
public:
	AssertionError(const SourceLocation &sourceLocation, const std::string &message)
	:
		message_(message + " at " + str(sourceLocation))
	{
	}

	virtual const char *what() const
	{
		return message_.c_str();
	}
private:
	std::string message_;
};

static // TODO
int assertions = 0;

inline int flushAssertions()
{
	int previous = assertions;
	assertions = 0;
	return previous;
}

inline void assertTrue(const SourceLocation &sourceLocation, bool expression, const std::string &message)
{
	++assertions;
	if(!expression)
	{
		throw AssertionError(sourceLocation, message);
	}
}
#define assertTrue(EXPRESSION, MESSAGE) assertTrue(CURRENT_SOURCE_LOCATION, EXPRESSION, MESSAGE)

template <typename X, typename Y>
void assertEquals(const SourceLocation &sourceLocation, const X &x, const Y &y)
{
	++assertions;
	if (x != y)
	{
		throw AssertionError(sourceLocation, "'" + str(x) + "' should equal '" + str(y) + "'");
	}
}
#define assertEquals(X, Y) assertEquals(CURRENT_SOURCE_LOCATION, X, Y)

#endif

