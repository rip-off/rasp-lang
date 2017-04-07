#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>
#include <vector>
#include <stdexcept>
#include "source_location.h"

class RaspError : public std::runtime_error
{
public:
	struct StackElement {
		SourceLocation sourceLocation;
		std::string message;
	};
	typedef std::vector<StackElement> StackTrace;

	RaspError(const std::string &message, const SourceLocation &sourceLocation);
	virtual ~RaspError();
	const StackTrace &stacktrace() const;
	void buildStackTrace(const std::string &message, const SourceLocation &sourceLocation);

private:
	StackTrace stacktrace_;
};

void printStackTrace(std::ostream &out, const RaspError &e);

#endif
