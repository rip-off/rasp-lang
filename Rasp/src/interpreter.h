#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "settings.h"
#include "function.h"
#include "bindings.h"
#include "instruction.h"

class Interpreter
{
public:
	Interpreter(const Bindings &bindings, const Settings &settings);

	Value exec(const InstructionList &instructions);

	Value exec(const InstructionList &instructions, Bindings &bindings);

	const Value *binding(const Identifier &name) const;

	Bindings &bindings()
	{
		return bindings_;
	}

	const Bindings &bindings() const
	{
		return bindings_;
	}

	std::vector<Identifier> declarations() const;

private:
	Bindings bindings_;
	Settings settings_;
};

#endif
