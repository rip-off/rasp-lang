#include "instruction.h"
#include "utils.h"
#include "bug.h"

Instruction::Instruction(const SourceLocation &sourceLocation, Type type, const Value &value)
	: type_(type), value_(value), sourceLocation_(sourceLocation)
{
}

Instruction Instruction::push(const SourceLocation &sourceLocation, const Value &value)
{
	return Instruction(sourceLocation, PUSH, value);
}

Instruction Instruction::call(const SourceLocation &sourceLocation, int argc)
{
	return Instruction(sourceLocation, CALL, Value::number(argc));
}

Instruction Instruction::jump(const SourceLocation &sourceLocation, int instructions)
{
	return Instruction(sourceLocation, JUMP, Value::number(instructions));
}

Instruction Instruction::loop(const SourceLocation &sourceLocation, int instructions)
{
	return Instruction(sourceLocation, LOOP, Value::number(instructions));
}

Instruction Instruction::close(const SourceLocation &sourceLocation, int argc)
{
	return Instruction(sourceLocation, CLOSE, Value::number(argc));
}

Instruction Instruction::condJump(const SourceLocation &sourceLocation, int instructions)
{
	return Instruction(sourceLocation, COND_JUMP, Value::number(instructions));
}

Instruction Instruction::refLocal(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, REF_LOCAL, Value::string(identifier.name()));
}

Instruction Instruction::initLocal(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, INIT_LOCAL, Value::string(identifier));
}

Instruction Instruction::assignLocal(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, ASSIGN_LOCAL, Value::string(identifier));
}

Instruction Instruction::refGlobal(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, REF_GLOBAL, Value::string(identifier.name()));
}

Instruction Instruction::initGlobal(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, INIT_GLOBAL, Value::string(identifier));
}

Instruction Instruction::assignGlobal(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, ASSIGN_GLOBAL, Value::string(identifier));
}

Instruction Instruction::refClosure(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, REF_CLOSURE, Value::string(identifier.name()));
}

Instruction Instruction::assignClosure(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, ASSIGN_CLOSURE, Value::string(identifier));
}

Instruction Instruction::memberAccess(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, MEMBER_ACCESS, Value::string(identifier));
}

Instruction::Type Instruction::type() const
{
	return type_;
}

const Value &Instruction::value() const
{
	return value_;
}

const SourceLocation &Instruction::sourceLocation() const
{
	return sourceLocation_;
}

std::ostream &operator<<(std::ostream &out, const Instruction &instruction)
{
	switch(instruction.type_)
	{
	case Instruction::PUSH:
		return out << "push(" << instruction.value_ << ")";
	case Instruction::CALL:
		return out << "call(" << instruction.value_ << ")";
	case Instruction::JUMP:
		return out << "jump(" << instruction.value_ << ")";
	case Instruction::LOOP:
		return out << "loop(" << instruction.value_ << ")";
	case Instruction::CLOSE:
		return out << "close(" << instruction.value_ << ")";
	case Instruction::COND_JUMP:
		return out << "cond_jump(" << instruction.value_ << ")";
	case Instruction::REF_LOCAL:
		return out << "ref_local(" << instruction.value_.string() << ")";
	case Instruction::INIT_LOCAL:
		return out << "init_local(" << instruction.value_.string() << ")";
	case Instruction::ASSIGN_LOCAL:
		return out << "assign_local(" << instruction.value_.string() << ")";
	case Instruction::REF_GLOBAL:
		return out << "ref_global(" << instruction.value_.string() << ")";
	case Instruction::INIT_GLOBAL:
		return out << "init_global(" << instruction.value_.string() << ")";
	case Instruction::ASSIGN_GLOBAL:
		return out << "assign_global(" << instruction.value_.string() << ")";
	case Instruction::REF_CLOSURE:
		return out << "ref_closure(" << instruction.value_.string() << ")";
	case Instruction::ASSIGN_CLOSURE:
		return out << "assign_closure(" << instruction.value_.string() << ")";
	case Instruction::MEMBER_ACCESS:
		return out << "member(" << instruction.value_.string() << ")";
	default:
		throw CompilerBug("unhandled instruction type: " + str(instruction.type_));
	}
}
