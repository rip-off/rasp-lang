#ifndef BUG_H
#define BUG_H

#include <stdexcept>

class CompilerBug : public std::runtime_error
{
public:
	CompilerBug(const std::string &message) : std::runtime_error(message)
	{
	}
};

#endif

