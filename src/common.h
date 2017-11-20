#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include "value.h"
#include "bindings.h"

typedef std::vector<Value> Arguments;

class Interpreter;

class CallContext
{
	typedef Bindings::Mapping Globals;
public:
	CallContext(Globals *globals, Arguments *arguments, Interpreter *);

	const Arguments &arguments() const;
	Globals &globals();
	Interpreter &interpreter();

private:
	Globals *globals_;
	Arguments *arguments_;
	Interpreter *interpreter_;
};


#endif
