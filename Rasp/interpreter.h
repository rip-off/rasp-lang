#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "function.h"
#include "bindings.h"
#include "instruction.h"

class Interpreter
{
public:
	Interpreter(const Bindings &bindings)
		: bindings(bindings)
	{
	}

	Value exec(const InstructionList &instructions) const;

	const Value *binding(const std::string &name) const;

private:
	Bindings bindings;
};

#endif
