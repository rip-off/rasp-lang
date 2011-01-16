#ifndef API_H
#define API_H

#include <vector>
#include <algorithm>

#include "utils.h"
#include "common.h"
#include "bindings.h"
#include "function.h"

struct ApiReg
{
public:
	ApiReg(const std::string &name, ApiFunction *function) 
		: name_(name), function_(function)
	{
	}

	const std::string &name() const 
	{ 
		return name_; 
	}

	ApiFunction *function() const
	{ 
		return function_; 
	}

private:
	std::string name_;
	ApiFunction *function_;
};

template<int N>
void registerBindings(Bindings &bindings, const ApiReg (&registry)[N])
{
	for(const ApiReg *current = registry ; current != registry + N ; ++current)
	{
		Function function(current->name(), current->function());
		bindings.insert(std::make_pair(function.name(), function));
	}
}

#endif
