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
	CallContext(Bindings::Mapping *globals, const Arguments &, Interpreter *);

	CallContext(Bindings::Mapping *globals, const Bindings::Mapping &closedValues, const Arguments &, Interpreter *);

	const Arguments &arguments() const;

	// TODO: pointer?
	Bindings::Mapping &globals();

	const Bindings::Mapping &closedValues() const;

	Interpreter *interpreter();

private:
	Bindings::Mapping *globals_;
	Bindings::Mapping closedValues_;
	Arguments arguments_;
	Interpreter *interpreter_;
};


#endif
