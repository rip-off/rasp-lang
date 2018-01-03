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
		CALL,
		PUSH,
		JUMP,
		LOOP,
		CLOSE,
		COND_JUMP,
		REF_LOCAL,
		INIT_LOCAL,
		ASSIGN_LOCAL,
		REF_GLOBAL,
		INIT_GLOBAL,
		ASSIGN_GLOBAL,
		REF_CLOSURE,
		INIT_CLOSURE,
		ASSIGN_CLOSURE,
		MEMBER_ACCESS,
	};

	static Instruction push(const SourceLocation &sourceLocation, const Value &value);

	static Instruction call(const SourceLocation &sourceLocation, int argc);

	static Instruction loop(const SourceLocation &sourceLocation, int instructions);

	static Instruction jump(const SourceLocation &sourceLocation, int instructions);

	static Instruction close(const SourceLocation &sourceLocation, int argc);

	static Instruction condJump(const SourceLocation &sourceLocation, int instructions);
	
	static Instruction refLocal(const SourceLocation &sourceLocation, const Identifier &identifier);

	static Instruction initLocal(const SourceLocation &sourceLocation, const Identifier &identifier);

	static Instruction assignLocal(const SourceLocation &sourceLocation, const Identifier &identifier);

	static Instruction refGlobal(const SourceLocation &sourceLocation, const Identifier &identifier);
	
	static Instruction initGlobal(const SourceLocation &sourceLocation, const Identifier &identifier);

	static Instruction assignGlobal(const SourceLocation &sourceLocation, const Identifier &identifier);

	static Instruction refClosure(const SourceLocation &sourceLocation, const Identifier &identifier);

	static Instruction initClosure(const SourceLocation &sourceLocation, const Identifier &identifier);
	
	static Instruction assignClosure(const SourceLocation &sourceLocation, const Identifier &identifier);

	// TODO: Identifier
	static Instruction memberAccess(const SourceLocation &sourceLocation, const std::string &identifier);

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
