#include "interpreter.h"

#include <stack>

#include "api.h"
#include "bug.h"
#include "execution_error.h"
#include "closure.h"

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

	Value handleFunction(Interpreter *interpreter, const Value &value, Stack &stack, Bindings &bindings)
	{
		if(!value.isNumber())
		{
			throw CompilerBug("Call instruction expects a numeric argument");				
		}
		// TODO: signed/unsigned mismatch...
		unsigned argc = value.number();
		if(stack.size() < argc + 1)
		{
			// TODO: include stack size and argc in exception information.
			throw CompilerBug("Not enough values on the stack to call function");
		}
			
		Value top = pop(stack);
		if(!top.isFunction())
		{
			throw CompilerBug("Call instruction expects top of the stack to be functional value");
		}

		Arguments arguments;
		while(argc --> 0)
		{
			arguments.push_back(pop(stack));
		}

		const Function &function = top.function();
		try
		{
			CallContext callContext(&bindings, &arguments, interpreter);
			return function.call(callContext);
		}
		catch (RaspError &error)
		{
			error.buildStackTrace(" at function: " + function.name(), function.sourceLocation());
			throw;
		}
	}

	Value handleCapture(Interpreter *interpreter, const Value &value, Stack &stack, Bindings &bindings)
	{
		if(!value.isNumber())
		{
			throw CompilerBug("Capture instruction expects a numeric argument");				
		}
		// TODO: signed/unsigned mismatch...
		unsigned argc = value.number();
		if(stack.size() < argc + 1)
		{
			// TODO: include stack size and argc in exception information.
			throw CompilerBug("Not enough values on the stack to capture closure");
		}
			
		Value top = pop(stack);
		if(!top.isFunction())
		{
			throw CompilerBug("Capture instruction expects top of the stack to be functional value");
		}

		Arguments closedValues;
		while(argc --> 0)
		{
			closedValues.push_back(pop(stack));
		}

		Closure closure(top.function(), closedValues);
		return Value::function(closure);
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
	return exec(instructions, bindings_);
}

Value Interpreter::exec(const InstructionList &instructions, Bindings &bindings)
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
				std::cout << "DEBUG: " << it->sourceLocation() << " noop!\n";
			}
			// Do nothing
			break;
		case Instruction::RefLocal: // TODO: de-duplicate?
			{
				// TODO: compiler bug: binding not found?
				Identifier identifier = Identifier(value.string());
				const Value &value = bindings[identifier];
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " local " << identifier.name() << " = " << value << '\n';
				}
				stack.push_back(value);
			}
			break;
		case Instruction::RefGlobal:
			{
				// TODO: compiler bug: binding not found?
				Identifier identifier = Identifier(value.string());
				const Value &value = bindings[identifier];
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " global " << identifier.name() << " = " << value << '\n';
				}
				stack.push_back(value);
			}
			break;
		case Instruction::RefClosure:
			{
				// TODO: compiler bug: binding not found?
				Identifier identifier = Identifier(value.string());
				const Value &value = bindings[identifier];
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " closure " << identifier.name() << " = " << value << '\n';
				}
				stack.push_back(value);
			}
			break;
		case Instruction::Push:
			if(settings_.trace)
			{				
				std::cout << "DEBUG: " << it->sourceLocation() << " push " << value << '\n';
			}
			stack.push_back(value);
			break;
		case Instruction::Call:
			{
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " call " << value << '\n';
				}
				// TODO: member function?
				Value result = handleFunction(this, value, stack, bindings);
				stack.push_back(result);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " return value " << result << '\n';
				}
			}
			break;
		case Instruction::Jump:
			{
				if(stack.empty())
				{
					throw CompilerBug("empty stack when testing condition");
				}
				int instructionsToSkip = value.number();
				int remaining = instructions.end() - it;
				if(remaining < instructionsToSkip)
				{
					throw CompilerBug("insufficient instructions available to skip!");
				}

				Value top = pop(stack);
				if(settings_.trace)
				{				
					std::cout << "DEBUG: " << it->sourceLocation() << " jumping back " << instructionsToSkip << " if " << top << '\n';
				}

				if(!top.asBool())
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
					throw CompilerBug("insufficient instructions available to loop!");
				}

				if(settings_.trace)
				{				
					std::cout << "DEBUG: " << it->sourceLocation() << " looping back " << instructionCount << " instructions\n";
				}
				it -= instructionCount;
			}
			break;
		case Instruction::Assign:
			{
				if(stack.empty())
				{
					throw CompilerBug("empty stack during assignment");
				}
				assert(value.isString());
				// Don't pop, allows this to be nested in larger statements
				// e.g. ((defun foo () ...))
				Value top = stack.back();
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " assigning " << value.string() << " to " << top << '\n';
				}
				Identifier identifier = Identifier(value.string());
				bindings[identifier] = top;
			}
			break;
		case Instruction::Capture:
			{
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " capture " << value << '\n';
				}
				// TODO: member function?
				Value result = handleCapture(this, value, stack, bindings);
				stack.push_back(result);
			}
			break;
		default:
			throw CompilerBug("unhandled instruction type: " + str(type));
		}

		if (settings_.trace)
		{
			if (stack.empty())
			{
				std::cout << "Stack is empty\n";
			}
			else
			{
				std::cout << "Stack contains " << stack.size() << " entries:\n";
				int index = 0;
				for(Stack::const_iterator it = stack.begin() ; it != stack.end() ; ++it)
				{
					++index;
					std::cout << index << ":  " << *it << '\n';
				}
			}
		}
	}
	
	return stack.empty() ? Value::nil() : pop(stack);
}

Declarations Interpreter::declarations() const
{
	return Declarations(bindings_);
}

const Value *Interpreter::binding(const Identifier &name) const
{
	return tryFind(bindings_, name);
}

