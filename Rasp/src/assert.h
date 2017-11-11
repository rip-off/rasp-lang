#ifndef ASSERT_H
#define ASSERT_H

#include "source_location.h"



class AssertionError
{
public:
	AssertionError(const SourceLocation &sourceLocation, const std::string &message);

	virtual const char *what() const;
private:
	std::string message_;
};

void incrementAssertions();
int flushAssertions();

void assertTrue(const SourceLocation &sourceLocation, bool expression, const std::string &message);

template <typename X, typename Y>
void assertEquals(const SourceLocation &sourceLocation, const X &x, const Y &y)
{
	incrementAssertions();
	if (x != y)
	{
		throw AssertionError(sourceLocation, "'" + str(x) + "' should equal '" + str(y) + "'");
	}
}

#ifdef RASP_ENABLE_ASSERTION_MACROS
#define assertTrue(EXPRESSION, MESSAGE) assertTrue(CURRENT_SOURCE_LOCATION, EXPRESSION, MESSAGE)
#define CURRENT_SOURCE_LOCATION SourceLocation(__FILE__, __LINE__)
#define assertEquals(X, Y) assertEquals(CURRENT_SOURCE_LOCATION, X, Y)
#endif

#endif

