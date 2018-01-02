#include "common.h"

CallContext::CallContext(
	Bindings::Mapping *globals,
	const Arguments &arguments,
	Interpreter *interpreter)
:
	globals_(globals),
	arguments_(arguments),
	interpreter_(interpreter)
{
}

CallContext::CallContext(
	Bindings::Mapping *globals,
	const Bindings::Mapping &closedValues,
	const Arguments &arguments,
	Interpreter *interpreter)
:
	globals_(globals),
	closedValues_(closedValues),
	arguments_(arguments),
	interpreter_(interpreter)
{
}

const Arguments &CallContext::arguments() const
{
	return arguments_;
}

Bindings::Mapping &CallContext::globals()
{
	return *globals_;
}

const Bindings::Mapping &CallContext::closedValues() const
{
	return closedValues_;
}

Interpreter *CallContext::interpreter()
{
	return interpreter_;
}

