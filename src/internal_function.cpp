#include "internal_function.h"
#include "interpreter.h"
#include "utils.h"
#include "exceptions.h"
#include "execution_error.h"

InternalFunction::InternalFunction(
	const SourceLocation &sourceLocation,
	const std::string &name, 
	const std::vector<Identifier> &parameters,
	const InstructionList &instructionList)
:
	sourceLocation_(sourceLocation),
	name_(name),
	parameters_(parameters),
	instructionList_(instructionList)
{
}

Function *InternalFunction::clone() const
{
	return new InternalFunction(sourceLocation_, name_, parameters_, instructionList_);
}

Value InternalFunction::call(CallContext &callContext) const
{
	const Arguments &arguments = callContext.arguments();
	if (arguments.size() != parameters_.size())
	{
		throw ExecutionError(sourceLocation_, "Function '" + name_ + "' passed " + str(arguments.size()) + " arguments but expected " + str(parameters_.size()));
	}
	Bindings localBindings(&callContext.globals());
	for (unsigned i = 0 ; i < parameters_.size() ; ++i)
	{
		localBindings.initLocal(parameters_[i], arguments[i]);
	}
	return callContext.interpreter().exec(instructionList_, localBindings);
}

const std::string &InternalFunction::name() const
{
	return name_;
}

const SourceLocation &InternalFunction::sourceLocation() const
{
	return sourceLocation_;
}

