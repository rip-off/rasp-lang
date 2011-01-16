#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <vector>

#include "value.h"
#include "function.h"

class Instruction
{
public:
	enum Type
	{
		Call,
		Push,
		NoOp,
	};

	static Instruction noop()
	{
		return Instruction(NoOp, Value::nil());
	}

	static Instruction push(const Value &value) 
	{
		return Instruction(Push, value);
	}

	static Instruction function(const Function &func) 
	{
		return Instruction(Push, func);
	}

	static Instruction call(int argc)
	{
		return Instruction(Call, argc);
	}

	Type type() const
	{
		return type_;
	}

	const Value &value() const
	{
		return value_;
	}

private:
	Instruction(Type type, const Value &value)
		: type_(type), value_(value)
	{
	}

	Type type_;
	Value value_;
};

typedef std::vector<Instruction> InstructionList;

#endif
