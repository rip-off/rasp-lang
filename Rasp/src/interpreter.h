#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "settings.h"
#include "function.h"
#include "bindings.h"
#include "instruction.h"

class Interpreter
{
public:
	typedef Bindings::Mapping Globals;

	Interpreter(const Globals &globals, const Settings &settings);

	Value exec(const InstructionList &instructions);

	Value exec(const InstructionList &instructions, Bindings &bindings);

	const Value *global(const Identifier &name) const;

	Declarations declarations() const;

private:
	Globals globals_;
	Settings settings_;
};

#endif
