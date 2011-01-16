#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>

#include "common.h"

class Function
{
public:
	Function(const std::string &name, ApiFunction *functionPointer)
		: name_(name), functionPointer(functionPointer)
	{
	}

	Value call(const Arguments &arguments) const;

	const std::string &name() const
	{
		return name_;
	}

private:
	std::string name_;
	ApiFunction *functionPointer;
};



#endif
