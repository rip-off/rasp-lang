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

Instruction Instruction::ref(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Instruction(sourceLocation, Ref, Value::string(identifier.name()));
}

Instruction Instruction::push(const SourceLocation &sourceLocation, const Value &value)
{
	return Instruction(sourceLocation, Push, value);
}

Instruction Instruction::function(const SourceLocation &sourceLocation, const Function &func)
{
	return Instruction(sourceLocation, Push, Value::function(func));
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

Instruction::Type Instruction::type() const
{
	return type_;
}

const Value &Instruction::value() const
{
	return value_;
}

std::ostream &operator<<(std::ostream &out, const Instruction &instruction)
{
	switch(instruction.type_)
	{
	case Instruction::NoOp:
		out << "noop";
		break;
	case Instruction::Ref:
		out << "ref(" << instruction.value_ << ")";
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
		out << "assign(" << instruction.value_ << ")";
		break;
	default:
		throw CompilerBug("unhandled instruction type: " + str(instruction.type_));
	}
	return out;
}
