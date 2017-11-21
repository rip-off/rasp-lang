#include "instruction.h"
#include "utils.h"
#include "bug.h"

Instruction::Instruction(const SourceLocation &sourceLocation, Type type, const Value &value)
	: type_(type), value_(value), sourceLocation_(sourceLocation)
{
}

Instruction Instruction::push(const SourceLocation &sourceLocation, const Value &value)
{
	return Instruction(sourceLocation, Push, value);
}

Instruction Instruction::call(const SourceLocation &sourceLocation, int argc)
{
	return Instruction(sourceLocation, Call, Value::number(argc));
}

Instruction Instruction::jump(const SourceLocation &sourceLocation, int instructions)
{
	return Instruction(sourceLocation, Jump, Value::number(instructions));
}

Instruction Instruction::loop(const SourceLocation &sourceLocation, int instructions)
{
	return Instruction(sourceLocation, Loop, Value::number(instructions));
}

Instruction Instruction::capture(const SourceLocation &sourceLocation, int argc)
{
	return Instruction(sourceLocation, Capture, Value::number(argc));
}

Instruction Instruction::condJump(const SourceLocation &sourceLocation, int instructions)
{
	return Instruction(sourceLocation, CondJump, Value::number(instructions));
}

Instruction Instruction::refLocal(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, RefLocal, Value::string(identifier.name()));
}

Instruction Instruction::initLocal(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, InitLocal, Value::string(identifier));
}

Instruction Instruction::assignLocal(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, AssignLocal, Value::string(identifier));
}

Instruction Instruction::refGlobal(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, RefGlobal, Value::string(identifier.name()));
}

Instruction Instruction::initGlobal(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, InitGlobal, Value::string(identifier));
}

Instruction Instruction::assignGlobal(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, AssignGlobal, Value::string(identifier));
}

Instruction Instruction::refClosure(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, RefClosure, Value::string(identifier.name()));
}

Instruction Instruction::assignClosure(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, AssignClosure, Value::string(identifier));
}

Instruction Instruction::memberAccess(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, MemberAccess, Value::string(identifier));
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
	case Instruction::Push:
		return out << "push(" << instruction.value_ << ")";
	case Instruction::Call:
		return out << "call(" << instruction.value_ << ")";
	case Instruction::Jump:
		return out << "jump(" << instruction.value_ << ")";
	case Instruction::Loop:
		return out << "loop(" << instruction.value_ << ")";
	case Instruction::Capture:
		return out << "capture(" << instruction.value_ << ")";
	case Instruction::CondJump:
		return out << "cond_jump(" << instruction.value_ << ")";
	case Instruction::RefLocal:
		return out << "ref_local(" << instruction.value_.string() << ")";
	case Instruction::InitLocal:
		return out << "init_local(" << instruction.value_.string() << ")";
	case Instruction::AssignLocal:
		return out << "assign_local(" << instruction.value_.string() << ")";
	case Instruction::RefGlobal:
		return out << "ref_global(" << instruction.value_.string() << ")";
	case Instruction::InitGlobal:
		return out << "init_global(" << instruction.value_.string() << ")";
	case Instruction::AssignGlobal:
		return out << "assign_global(" << instruction.value_.string() << ")";
	case Instruction::RefClosure:
		return out << "ref_closure(" << instruction.value_.string() << ")";
	case Instruction::AssignClosure:
		return out << "assign_closure(" << instruction.value_.string() << ")";
	case Instruction::MemberAccess:
		return out << "member(" << instruction.value_.string() << ")";
	default:
		throw CompilerBug("unhandled instruction type: " + str(instruction.type_));
	}
}
