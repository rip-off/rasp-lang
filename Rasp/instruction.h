#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <vector>
#include <iostream>

#include "value.h"
#include "function.h"

class Instruction
{
public:
	typedef std::vector<Instruction> InstructionList;

	enum Type
	{
		Call,
		Push,
		NoOp,
		Jump,
	};

	static Instruction noop();

	static Instruction push(const Value &value) ;

	static Instruction function(const Function &func);

	static Instruction call(int argc);

	static Instruction jump(int instructions);

	Type type() const;

	const Value &value() const;

	friend std::ostream &operator<<(std::ostream &out, const Instruction &);

private:
	Instruction(Type type, const Value &value);

	Type type_;
	Value value_;
};

typedef Instruction::InstructionList InstructionList;

#endif
