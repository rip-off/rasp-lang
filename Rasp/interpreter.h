#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "function.h"
#include "bindings.h"
#include "instruction.h"

class Interpreter
{
public:
	Interpreter(const Bindings &bindings)
		: bindings_(bindings)
	{
	}

	Value exec(const InstructionList &instructions) const;

	const Value *binding(const std::string &name) const;

	// TODO: remove this...
	const Bindings &bindings() const
	{
		return bindings_;
	}

private:
	Bindings bindings_;
};

#endif
