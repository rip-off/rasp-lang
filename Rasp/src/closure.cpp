#include "closure.h"

Closure::Closure(const Function &function, const Arguments &closedValues)
:
	innerFunction(function.clone()),
	closedValues(closedValues)
{
}

Closure *Closure::clone() const
{
	return new Closure(*innerFunction, closedValues);
}

Value Closure::call(CallContext &callContext) const
{
	const Arguments &passedArguments = callContext.arguments();
	Arguments allArguments;
	allArguments.reserve(closedValues.size() + passedArguments.size());
	allArguments.insert(allArguments.end(), closedValues.begin(), closedValues.end());
	allArguments.insert(allArguments.end(), passedArguments.begin(), passedArguments.end());
	CallContext nestedContext(&callContext.bindings(), &allArguments, &callContext.interpreter());
	return innerFunction->call(nestedContext);
}

const std::string &Closure::name() const
{
	return innerFunction->name();
}

const SourceLocation &Closure::sourceLocation() const
{
	return innerFunction->sourceLocation();
}

