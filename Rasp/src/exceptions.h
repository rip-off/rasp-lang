#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>
#include <vector>
#include <stdexcept>

class RaspError : public std::runtime_error
{
public:
	typedef std::vector<std::string> StackTrace;

	RaspError(const std::string &message)
		: std::runtime_error(message)
	{
	}

	virtual ~RaspError();

	const StackTrace &stacktrace() const;

	void buildStackTrace(const std::string &entry);

private:
	RaspError(const RaspError &);
	RaspError &operator=(const RaspError &);

	StackTrace stacktrace_;
};

void printStackTrace(std::ostream &out, const RaspError &e);

#endif
