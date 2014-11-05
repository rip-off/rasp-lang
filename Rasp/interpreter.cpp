#include "interpreter.h"

#include <stack>

#include "api.h"
#include "exceptions.h"

namespace
{
	typedef std::vector<Value> Stack;

	Value pop(Stack &stack)
	{
		assert(!stack.empty());
		Value result = stack.back();
		stack.pop_back();
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
		stack.push_back(result);
	}
}

Interpreter::Interpreter(const Bindings &bindings, const Settings &settings)
:
	bindings_(bindings),
	settings_(settings)
{
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
			if(settings_.trace)
			{				
				std::cout << "DEBUG: noop!\n";
			}
			// Do nothing
			break;
		case Instruction::Ref:
			{
				// TODO: compiler bug: binding not found?
				Identifier identifier = Identifier(value.string());
				const Value &value = bindings_[identifier];
				if(settings_.trace)
				{				
					std::cout << "DEBUG: ref " << identifier.name() << "(" << value << ")" << '\n';
				}
				stack.push_back(value);
			}
			break;
		case Instruction::Push:
			if(settings_.trace)
			{				
				std::cout << "DEBUG: push " << value << '\n';
			}
			stack.push_back(value);
			break;
		case Instruction::Call:
			if(settings_.trace)
			{				
				std::cout << "DEBUG: call " << value << '\n';
			}
			handleFunction(value, stack, bindings_);
			break;
		case Instruction::Jump:
			{
				if(stack.empty())
				{
					throw std::logic_error("Compiler bug: empty stack when testing condition");
				}
				int instructionsToSkip = value.number();
				int remaining = instructions.end() - it;
				if(remaining < instructionsToSkip)
				{
					throw std::logic_error("Compiler bug: insufficient instructions available to skip!");
				}

				Value top = pop(stack);
				if(settings_.trace)
				{				
					std::cout << "DEBUG: jumping back " << instructionsToSkip << " if " << top << '\n';
				}
				if(top.isNil())
				{
					it += instructionsToSkip;
				}
			}
			break;
		case Instruction::Loop:
			{
				int instructionCount = value.number();
				if(instructionCount > instructions.size())
				{
					throw std::logic_error("Compiler bug: insufficient instructions available to loop!");
				}

				if(settings_.trace)
				{				
					std::cout << "DEBUG: looping back " << instructionCount << " instructions\n";
				}
				it -= instructionCount;
			}
			break;
		case Instruction::Assign:
			{
				if(stack.empty())
				{
					throw std::logic_error("Compiler bug: empty stack during assignment");
				}
				assert(value.isString());
				Value top = pop(stack);
				if(settings_.trace)
				{
					std::cout << "DEBUG: assigning " << value.string() << " to " << top << '\n';
				}
				Identifier identifier = Identifier(value.string());
				bindings_[identifier] = top;
			}
			break;
		default:
			throw std::logic_error("Compiler bug: unhandled instruction type: " + str(type));
		}

		if (settings_.trace && !stack.empty())
		{
			std::cout << "Stack contains " << stack.size() << " entries:\n";
			for(Stack::const_iterator it = stack.begin() ; it != stack.end() ; ++it)
			{
				std::cout << " * " << *it << '\n';
			}
		}
	}
	
	return stack.empty() ? Value::nil() : pop(stack);
}

std::vector<Identifier> Interpreter::declarations() const
{
	std::vector<Identifier> result;
	for(Bindings::const_iterator it = bindings_.begin() ; it != bindings_.end() ; ++it)
	{
		result.push_back(it->first);
	}
	return result;
}

const Value *Interpreter::binding(const Identifier &name) const
{
	return tryFind(bindings_, name);
}

