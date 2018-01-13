#include "closure.h"

Closure::Closure(const Function &function, const Bindings::Mapping &closedValuesByName)
:
	innerFunction_(function.clone()),
	closedValuesByName_(closedValuesByName)
{
}

Closure *Closure::clone() const
{
	return new Closure(*innerFunction_, closedValuesByName_);
}

Value Closure::call(CallContext &callContext) const
{
	CallContext nestedContext(callContext.globals(), closedValuesByName_, callContext.arguments(), callContext.interpreter());
	return innerFunction_->call(nestedContext);
}

const std::string &Closure::name() const
{
	return innerFunction_->name();
}

const SourceLocation &Closure::sourceLocation() const
{
	return innerFunction_->sourceLocation();
}

