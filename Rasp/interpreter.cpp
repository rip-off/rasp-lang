#include "interpreter.h"

#include <stack>

#include "api.h"
#include "exceptions.h"

namespace
{
	typedef std::stack<Value> Stack;

	Value pop(Stack &stack)
	{
		Value result = stack.top();
		stack.pop();
		return result;
	}

	void handleFunction(const Value &value, Stack &stack)
	{
		if(!value.isNumber())
		{
			throw ExecutionError("Call instruction expects a numeric argument");				
		}
		// TODO: signed/unsigned mismatch...
		unsigned argc = value.number();
		if(stack.size() < argc + 1)
		{
			// TODO: include stack size and argc in exception information.
			throw ExecutionError("Not enough values on the stack to call function");
		}
			
		Value top = pop(stack);
		if(!top.isFunction())
		{
			throw ExecutionError("Call instruction expects top of the stack to be functional value");
		}
		Arguments arguments;
		while(argc --> 0)
		{
			arguments.push_back(pop(stack));
		}
		const Function &function = top.function();
		Value result = function.call(arguments);
		stack.push(result);
	}
}

Value Interpreter::exec(const InstructionList &instructions) const
{
	Stack stack;
	for(InstructionList::const_iterator it = instructions.begin() ; it != instructions.end() ; ++it)
	{
		Instruction::Type type = it->type();
		const Value &value = it->value();
		switch(type)
		{
		case Instruction::NoOp:
			// Do nothing
			break;
		case Instruction::Push:
			stack.push(value);
			break;
		case Instruction::Call:
			handleFunction(value, stack);
			break;
		}
	}
	return stack.empty() ? Value::nil() : pop(stack);
}

const Value *Interpreter::binding(const std::string &name) const
{
	return tryFind(bindings_, name);
}
