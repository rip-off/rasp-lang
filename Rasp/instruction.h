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
		Ref,
		Call,
		Push,
		NoOp,
		Jump,
		Loop,
		Assign,
	};

	static Instruction noop();

	static Instruction ref(const Identifier &identifier);

	static Instruction push(const Value &value);

	static Instruction function(const Function &func);

	static Instruction call(int argc);

	static Instruction jump(int instructions);

	static Instruction loop(int instructions);

	static Instruction assign(const std::string &identifier);

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
