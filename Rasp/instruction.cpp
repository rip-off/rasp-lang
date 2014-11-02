#include "instruction.h"

Instruction::Instruction(Type type, const Value &value)
	: type_(type), value_(value)
{
}

Instruction Instruction::noop()
{
	return Instruction(NoOp, Value::nil());
}

Instruction Instruction::push(const Value &value) 
{
	return Instruction(Push, value);
}

Instruction Instruction::function(const Function &func) 
{
	return Instruction(Push, func);
}

Instruction Instruction::call(int argc)
{
	return Instruction(Call, argc);
}

Instruction Instruction::jump(int instructions)
{
	return Instruction(Jump, instructions);
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
	case Instruction::Push:
		out << "push(" << instruction.value_ << ")";
		break;
	case Instruction::Call:
		out << "call(" << instruction.value_ << ")";
		break;
	case Instruction::Jump:
		out << "jump(" << instruction.value_ << ")";
		break;
	default:
		throw std::logic_error("Compiler bug: unhandled instruction type!");
	}
	return out;
}
