#include "common.h"

CallContext::CallContext(Bindings *bindings, Arguments *arguments)
:
	bindings_(bindings),
	arguments_(arguments)
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

