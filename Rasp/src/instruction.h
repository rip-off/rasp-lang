#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <vector>
#include <iostream>

#include "value.h"
#include "function.h"
#include "source_location.h"

class Instruction
{
public:
	typedef std::vector<Instruction> InstructionList;

	enum Type
	{
		RefLocal,
		RefGlobal,
		RefClosure,
		Call,
		Push,
		NoOp,
		Jump,
		Loop,
		Assign,
		Capture,
	};

	static Instruction noop(const SourceLocation &sourceLocation);

	static Instruction refLocal(const SourceLocation &sourceLocation, const Identifier &identifier);

	static Instruction refGlobal(const SourceLocation &sourceLocation, const Identifier &identifier);

	static Instruction refClosure(const SourceLocation &sourceLocation, const Identifier &identifier);

	static Instruction push(const SourceLocation &sourceLocation, const Value &value);

	static Instruction call(const SourceLocation &sourceLocation, int argc);

	static Instruction jump(const SourceLocation &sourceLocation, int instructions);

	static Instruction loop(const SourceLocation &sourceLocation, int instructions);

	static Instruction assign(const SourceLocation &sourceLocation, const std::string &identifier);

	static Instruction capture(const SourceLocation &sourceLocation, int argc);

	Type type() const;

	const Value &value() const;

	const SourceLocation &sourceLocation() const;

	friend std::ostream &operator<<(std::ostream &out, const Instruction &);

private:
	Instruction(const SourceLocation &sourceLocation, Type type, const Value &value);

	Type type_;
	Value value_;
	SourceLocation sourceLocation_;
};

typedef Instruction::InstructionList InstructionList;

#endif
