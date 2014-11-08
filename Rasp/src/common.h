#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include "value.h"
#include "bindings.h"

typedef std::vector<Value> Arguments;

class CallContext
{
public:
	CallContext(Bindings *bindings, Arguments *arguments);

	const Arguments &arguments() const;
	Bindings &bindings();

private:
	Bindings *bindings_;
	Arguments *arguments_;
};


#endif
