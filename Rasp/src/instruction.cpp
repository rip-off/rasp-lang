#include "instruction.h"
#include "utils.h"
#include "bug.h"

Instruction::Instruction(Type type, const Value &value)
	: type_(type), value_(value)
{
}

Instruction Instruction::noop()
{
	return Instruction(NoOp, Value::nil());
}

Instruction Instruction::ref(const Identifier &identifier)
{
	return Instruction(Ref, identifier.name());
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

Instruction Instruction::loop(int instructions)
{
	return Instruction(Loop, instructions);
}

Instruction Instruction::assign(const std::string &identifier)
{
	return Instruction(Assign, identifier);
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
