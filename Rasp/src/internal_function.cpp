#include "internal_function.h"
#include "interpreter.h"

InternalFunction::InternalFunction(const std::string &name, const InstructionList &instructionList)
:
	name_(name),
	instructionList_(instructionList)
{
}

Function *InternalFunction::clone() const
{
	return new InternalFunction(name_, instructionList_);
}

Value InternalFunction::call(CallContext &callContext) const
{
	return callContext.interpreter().exec(instructionList_);
}

const std::string &InternalFunction::name() const
{
	return name_;
}

