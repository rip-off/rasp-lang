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

	void handleFunction(const Value &value, Stack &stack, Bindings &bindings)
	{
		if(!value.isNumber())
		{
			throw std::logic_error("Compiler bug: Call instruction expects a numeric argument");				
		}
		// TODO: signed/unsigned mismatch...
		unsigned argc = value.number();
		if(stack.size() < argc + 1)
		{
			// TODO: include stack size and argc in exception information.
			throw std::logic_error("Compiler bug: Not enough values on the stack to call function");
		}
			
		Value top = pop(stack);
		if(!top.isFunction())
		{
			throw std::logic_error("Compiler bug: Call instruction expects top of the stack to be functional value");
		}
		Arguments arguments;
		while(argc --> 0)
		{
			arguments.push_back(pop(stack));
		}
		const Function &function = top.function();
		CallContext callContext(&bindings, &arguments);
		Value result = function.call(callContext);
		stack.push(result);
	}
}

Value Interpreter::exec(const InstructionList &instructions)
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
			handleFunction(value, stack, bindings_);
			break;
		case Instruction::Jump:
			{
				if(stack.empty())
				{
					throw std::logic_error("Compiler bug: empty stack");
				}
				int instructionsToSkip = value.number();
				int remaining = instructions.end() - it;
				if(remaining < instructionsToSkip)
				{
					throw std::logic_error("Compiler bug: insufficient instructions available to skip!");
				}

				if(stack.top().isNil())
				{
					it += instructionsToSkip;
				}
			}
			break;
		default:
			throw std::logic_error("Compiler bug: unhandled instruction type!");
		}
	}
	return stack.empty() ? Value::nil() : pop(stack);
}

const Value *Interpreter::binding(const std::string &name) const
{
	return tryFind(bindings_, name);
}
