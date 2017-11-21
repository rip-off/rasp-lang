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

	void handleRef(Bindings::RefType refType, const Value &value, Stack &stack, const Bindings &bindings)
	{
		Identifier identifier = Identifier(value.string());
		stack.push_back(bindings.get(refType, identifier));
	}

	Value handleInit(Bindings::RefType refType, const Value &value, Stack &stack, Bindings &bindings)
	{
		if(stack.empty())
		{
			throw CompilerBug("empty stack during " + str(refType) + " assignment");
		}
		assert(value.isString());
		// Don't pop, allows this to be nested in larger statements
		// e.g. ((defun foo () ...))
		Value top = stack.back();
		Identifier identifier = Identifier(value.string());
		bindings.init(refType, identifier, top);
		return top;
	}

	Value handleAssign(Bindings::RefType refType, const Value &value, Stack &stack, Bindings &bindings)
	{
		if(stack.empty())
		{
			throw CompilerBug("empty stack during " + str(refType) + " assignment");
		}
		assert(value.isString());
		// Don't pop, allows this to be nested in larger statements
		// e.g. ((defun foo () ...))
		Value top = stack.back();
		Identifier identifier = Identifier(value.string());
		bindings.set(refType, identifier, top);
		return top;
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
			throw CompilerBug("Need " + str(argc + 1) + " values on stack to call function, but only have " + str(stack.size()));
		}
			
		Value top = pop(stack);
		if(!top.isFunction())
		{
			// TODO: this is a runtime error, as we currently cannot prove this statically
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
			// TODO: where is nicer to get the globals from, bindings or interpreter?
			CallContext callContext(&bindings.globals(), &arguments, interpreter);
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
			throw CompilerBug("Need " + str(argc + 1) + " values on stack to capture closure, but only have " + str(stack.size()));
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

Interpreter::Interpreter(const Globals &globals, const Settings &settings)
:
	globals_(globals),
	settings_(settings)
{
}

Value Interpreter::exec(const InstructionList &instructions)
{
	Bindings bindings(&globals_);
	return exec(instructions, bindings);
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
		case Instruction::Loop:
			{
				int instructionsAvailable = instructions.size(); // Note: signed type is important!
				int instructionCount = value.number();
				if(instructionCount > 0)
				{
					if(instructionCount > instructionsAvailable)
					{
						throw CompilerBug("insufficient instructions available to loop! (instructionCount: " + str(instructionCount) + " > instructions.size(): " + str(instructions.size()) + ")");
					}
				}
				else
				{
					if(instructionsAvailable < instructionCount)
					{
						throw CompilerBug("insufficient instructions available to loop! (instructionCount: " + str(instructionCount) + " > instructions.size(): " + str(instructions.size()) + ")");
					}
				}

				if(settings_.trace)
				{				
					std::cout << "DEBUG: " << it->sourceLocation() << " looping back " << instructionCount << " instructions\n";
				}
				it -= instructionCount;
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
		case Instruction::CondJump:
			{
				if(stack.empty())
				{
					throw CompilerBug("empty stack when testing conditional jump");
				}
				int instructionsToSkip = value.number();
				int remaining = instructions.end() - it;
				if(instructionsToSkip < 0)
				{
					throw CompilerBug("Cannot jump backwards!");
				}
				else if(remaining < instructionsToSkip)
				{
					throw CompilerBug("insufficient instructions available to skip! (remaining: " + str(remaining) + " < instructionsToSkip: " + str(instructionsToSkip) + ")");
				}

				Value top = pop(stack);
				if(settings_.trace)
				{				
					std::cout << "DEBUG: " << it->sourceLocation() << " jumping back " << instructionsToSkip << " if " << top << '\n';
				}

				if(top.isFalsey())
				{
					it += instructionsToSkip;
				}
			}
			break;
		case Instruction::RefLocal:
			handleRef(Bindings::Local, value, stack, bindings);
			if(settings_.trace)
			{
				std::cout << "DEBUG: " << it->sourceLocation() << " local ref '" << value.string() << "' is " << stack.back() << '\n';
			}
			break;
		case Instruction::InitLocal:
			{
				const Value &intialisedValue = handleInit(Bindings::Local, value, stack, bindings);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " local init '" << value.string() << "' to " << intialisedValue << '\n';
				}
			}
			break;
		case Instruction::AssignLocal:
			{
				const Value &assignedValue = handleAssign(Bindings::Local, value, stack, bindings);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " local assign '" << value.string() << "' to " << assignedValue << '\n';
				}
			}
			break;
		case Instruction::RefGlobal:
			handleRef(Bindings::Global, value, stack, bindings);
			if(settings_.trace)
			{
				std::cout << "DEBUG: " << it->sourceLocation() << " global ref '" << value.string() << "' is " << stack.back() << '\n';
			}
			break;
		case Instruction::InitGlobal:
			{
				const Value &intialisedValue = handleInit(Bindings::Global, value, stack, bindings);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " global init '" << value.string() << "' to " << intialisedValue << '\n';
				}
			}
			break;
		case Instruction::AssignGlobal:
			{
				const Value &assignedValue = handleAssign(Bindings::Global, value, stack, bindings);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " global assign '" << value.string() << "' to " << assignedValue << '\n';
				}
			}
			break;
		case Instruction::RefClosure:
			handleRef(Bindings::Closure, value, stack, bindings);
			if(settings_.trace)
			{
				std::cout << "DEBUG: " << it->sourceLocation() << " closure ref '" << value.string() << "' is " << stack.back() << '\n';
			}
			break;
		case Instruction::AssignClosure:
			{
				// TODO:
				throw CompilerBug("unimplemented: AssignClosure");
			}
			break;
		case Instruction::MemberAccess:
			{
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " member access " << value << '\n';
				}
				// TODO: RASP_ASSERT (throws CompilerBug(__FILE__, __LINE__, ...)?
				assert(value.isString());
				const std::string &memberName = value.string();

				Value top = pop(stack);
				if(!top.isObject())
				{
					throw ExecutionError(it->sourceLocation(), "Member access instruction requires an object but got " + str(top));
				}
				const Value::Object &object = top.object();
				Value::Object::const_iterator memberIterator = object.find(memberName);
				if (memberIterator == object.end())
				{
					throw ExecutionError(it->sourceLocation(), "Unknown member name " + memberName + " for " + str(top));
				}
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " member access " << value.string() << "." << memberName << " was " << memberIterator->second << '\n';
				}
				stack.push_back(memberIterator->second);
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
	return Declarations(globals_);
}

const Settings &Interpreter::settings() const
{
    return settings_;
}

const Value *Interpreter::global(const Identifier &name) const
{
	Bindings::const_iterator i = globals_.find(name);
	return (i == globals_.end() ? nullptr : &(i->second));
}

