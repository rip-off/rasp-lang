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

	const Value *binding(const std::string &name) const;

	Bindings &bindings()
	{
		return bindings_;
	}

	const Bindings &bindings() const
	{
		return bindings_;
	}

private:
	Bindings bindings_;
	Settings settings_;
};

#endif
