#include "internal_function.h"
#include "interpreter.h"
#include "utils.h"
#include "exceptions.h"
#include "execution_error.h"

InternalFunction::InternalFunction(
	unsigned line,
	const std::string &name, 
	const std::vector<Identifier> &parameters,
	const InstructionList &instructionList)
:
	line_(line),
	name_(name),
	parameters_(parameters),
	instructionList_(instructionList)
{
}

Function *InternalFunction::clone() const
{
	return new InternalFunction(line_, name_, parameters_, instructionList_);
}

Value InternalFunction::call(CallContext &callContext) const
{
	const Arguments &arguments = callContext.arguments();
	if (arguments.size() != parameters_.size())
	{
		throw ExecutionError(line_, "Function '" + name_ + "' passed " + str(arguments.size()) + " arguments but expected " + str(parameters_.size()));
	}
	Bindings localBindings = callContext.bindings();
	for (unsigned i = 0 ; i < parameters_.size() ; ++i)
	{
		localBindings[parameters_[i]] = arguments[i];
		// TODO: adding debug / trace
		// std::cout << "DEBUG: setting parameter '" <<  parameters_[i].name() << "' to " << arguments[i] << '\n';
	}
	return callContext.interpreter().exec(instructionList_, localBindings);
}

const std::string &InternalFunction::name() const
{
	return name_;
}

