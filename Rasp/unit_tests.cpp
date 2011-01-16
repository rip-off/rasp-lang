#include "unit_tests.h"

#include <cassert>
#include "instruction.h"
#include "interpreter.h"

namespace
{
	void testInterpreter(Interpreter &interpreter)
	{
		InstructionList instructions;
		instructions.push_back(Instruction::push(42));
		instructions.push_back(Instruction::push(13));
		instructions.push_back(Instruction::push(16));
		const Value *value = interpreter.binding("+");
		assert(value);
		instructions.push_back(Instruction::push(*value));
		instructions.push_back(Instruction::call(3));
		Value result = interpreter.exec(instructions);
		assert(result.isNumber());
		assert(result.number() == (42 + 13 + 16));
	}
}

void runUnitTests(Interpreter &interpreter)
{
	testInterpreter(interpreter);
}
