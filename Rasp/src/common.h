#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include "value.h"
#include "bindings.h"

typedef std::vector<Value> Arguments;

class Interpreter;

class CallContext
{
public:
	CallContext(Bindings *bindings, Arguments *arguments, Interpreter *);

	const Arguments &arguments() const;
	Bindings &bindings();
	Interpreter &interpreter();

private:
	Bindings *bindings_;
	Arguments *arguments_;
	Interpreter *interpreter_;
};


#endif
