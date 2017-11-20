#include "common.h"

CallContext::CallContext(
	Globals *globals,
	Arguments *arguments, 
	Interpreter *interpreter)
:
	globals_(globals),
	arguments_(arguments),
	interpreter_(interpreter)
{
}

const Arguments &CallContext::arguments() const
{
	return *arguments_;
}

CallContext::Globals &CallContext::globals()
{
	return *globals_;
}

Interpreter &CallContext::interpreter()
{
	return *interpreter_;
}

