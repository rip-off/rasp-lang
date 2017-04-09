#include "instruction.h"
#include "utils.h"
#include "bug.h"

Instruction::Instruction(const SourceLocation &sourceLocation, Type type, const Value &value)
	: type_(type), value_(value), sourceLocation_(sourceLocation)
{
}

Instruction Instruction::noop(const SourceLocation &sourceLocation)
{
	return Instruction(sourceLocation, NoOp, Value::nil());
}

Instruction Instruction::refLocal(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, RefLocal, Value::string(identifier.name()));
}

Instruction Instruction::refGlobal(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, RefGlobal, Value::string(identifier.name()));
}

Instruction Instruction::refClosure(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, RefClosure, Value::string(identifier.name()));
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

Instruction Instruction::assign(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Instruction(sourceLocation, Assign, Value::string(identifier));
}

Instruction Instruction::capture(const SourceLocation &sourceLocation, int argc)
{
	return Instruction(sourceLocation, Capture, Value::number(argc));
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
	case Instruction::NoOp:
		out << "noop";
		break;
	case Instruction::RefLocal:
		out << "ref_local(" << instruction.value_.string() << ")";
		break;
	case Instruction::RefGlobal:
		out << "ref_global(" << instruction.value_.string() << ")";
		break;
	case Instruction::RefClosure:
		out << "ref_closure(" << instruction.value_.string() << ")";
		break;
	case Instruction::Push:
		out << "push(" << instruction.value_ << ")";
		break;
	case Instruction::Call:
		out << "call(" << instruction.value_ << ")";
		break;
	case Instruction::Jump:
		out << "jump(" << instruction.value_ << ")";
		break;
	case Instruction::Loop:
		out << "loop(" << instruction.value_ << ")";
		break;
	case Instruction::Assign:
		out << "assign(" << instruction.value_.string() << ")";
		break;
	case Instruction::Capture:
		out << "capture(" << instruction.value_ << ")";
		break;
	case Instruction::MemberAccess:
		out << "member(" << instruction.value_.string() << ")";
		break;
	default:
		throw CompilerBug("unhandled instruction type: " + str(instruction.type_));
	}
	return out;
}
