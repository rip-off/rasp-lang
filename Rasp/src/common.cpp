#include "common.h"

CallContext::CallContext(
	Bindings *bindings, 
	Arguments *arguments, 
	Interpreter *interpreter)
:
	bindings_(bindings),
	arguments_(arguments),
	interpreter_(interpreter)
{
}

const Arguments &CallContext::arguments() const
{
	return *arguments_;
}

Bindings &CallContext::bindings()
{
	return *bindings_;
}

Interpreter &CallContext::interpreter()
{
	return *interpreter_;
}

