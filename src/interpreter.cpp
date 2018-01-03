#include "interpreter.h"

#include <stack>

#include "api.h"
#include "bug.h"
#include "execution_error.h"
#include "closure.h"

namespace
{
	typedef Interpreter::Stack Stack;

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

	unsigned getArgumentCount(const Value &value)
	{
		if(!value.isNumber())
		{
			throw CompilerBug("Number expected");
		}

		int argc = value.number();
		if (argc < 0)
		{
			throw CompilerBug("Argument count should not be negative");
		}
		return argc;
	}

	typedef std::pair<Identifier, Bindings::ValuePtr> ClosedNameAndValue;
	typedef std::vector<ClosedNameAndValue> ClosureValues;

	Value handleClose(const Value &value, Stack &stack, ClosureValues &closureValues, Bindings &bindings)
	{
		unsigned argc = getArgumentCount(value);
		if(closureValues.size() < argc) // TODO:
		{
			throw CompilerBug("Need " + str(argc) + " values on stack to close over captured values, but only have " + str(closureValues.size()));
		}
			
		Value top = pop(stack);
		if(!top.isFunction())
		{
			throw CompilerBug("Close instruction expects top of the stack to be functional value");
		}

		Bindings::Mapping closedValuesByName;
		for (int i = 0 ; i < argc ; ++i)
		{
			auto &pair = closureValues.back();
			closedValuesByName[pair.first] = pair.second;
			closureValues.pop_back();
		}

		Closure closure(top.function(), closedValuesByName);
		return Value::function(closure);
	}

	int getInstructionsToSkip(Instruction::Type type, const Value &value)
	{
		int instructionCount = value.number();
		if(instructionCount <= 0)
		{
			throw CompilerBug(str(type) + " requires a positive number of instructions to skip");
		}
		return instructionCount;
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
	ClosureValues closureValues;

	for(InstructionList::const_iterator it = instructions.begin() ; it != instructions.end() ; ++it)
	{
		Instruction::Type type = it->type();
		const Value &value = it->value();
		switch(type)
		{
		case Instruction::PUSH:
			if(settings_.trace)
			{				
				std::cout << "DEBUG: " << it->sourceLocation() << " push " << value << '\n';
			}
			stack.push_back(value);
			break;
		case Instruction::CALL:
			{
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " call " << value << '\n';
				}
				Value result = handleFunction(it->sourceLocation(), value, stack, bindings);
				stack.push_back(result);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " return value " << result << '\n';
				}
			}
			break;
		case Instruction::JUMP:
			{
				int instructionsToSkip = getInstructionsToSkip(type, value);
				int remaining = instructions.end() - it;
				if(remaining < instructionsToSkip)
				{
					throw CompilerBug("insufficient instructions available to skip! (remaining: " + str(remaining) + " < instructionsToSkip: " + str(instructionsToSkip) + ")");
				}

				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " jumping back " << instructionsToSkip << '\n';
				}
				it += instructionsToSkip;
			}
			break;
		case Instruction::LOOP:
			{
				int instructionsToSkip = getInstructionsToSkip(type, value);
				int instructionsAvailable = instructions.size(); // Note: signed type is important!
				if(instructionsAvailable < instructionsToSkip)
				{
					throw CompilerBug("insufficient instructions available to loop! (instructionsToSkip: " + str(instructionsToSkip) + " > instructions.size(): " + str(instructions.size()) + ")");
				}

				if(settings_.trace)
				{				
					std::cout << "DEBUG: " << it->sourceLocation() << " looping back " << instructionsToSkip << " instructions\n";
				}
				it -= instructionsToSkip;
			}
			break;
		case Instruction::CLOSE:
			{
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " close " << value << '\n';
				}
				Value result = handleClose(value, stack, closureValues, bindings);
				stack.push_back(result);
			}
			break;
		case Instruction::COND_JUMP:
			{
				if(stack.empty())
				{
					throw CompilerBug("empty stack when testing conditional jump");
				}
				int instructionsToSkip = getInstructionsToSkip(type, value);
				int remaining = instructions.end() - it;
				if(remaining < instructionsToSkip)
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
		case Instruction::REF_LOCAL:
			handleRef(Bindings::Local, value, stack, bindings);
			if(settings_.trace)
			{
				std::cout << "DEBUG: " << it->sourceLocation() << " local ref '" << value.string() << "' is " << stack.back() << '\n';
			}
			break;
		case Instruction::INIT_LOCAL:
			{
				const Value &intialisedValue = handleInit(Bindings::Local, value, stack, bindings);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " local init '" << value.string() << "' to " << intialisedValue << '\n';
				}
			}
			break;
		case Instruction::ASSIGN_LOCAL:
			{
				const Value &assignedValue = handleAssign(Bindings::Local, value, stack, bindings);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " local assign '" << value.string() << "' to " << assignedValue << '\n';
				}
			}
			break;
		case Instruction::REF_GLOBAL:
			handleRef(Bindings::Global, value, stack, bindings);
			if(settings_.trace)
			{
				std::cout << "DEBUG: " << it->sourceLocation() << " global ref '" << value.string() << "' is " << stack.back() << '\n';
			}
			break;
		case Instruction::INIT_GLOBAL:
			{
				const Value &intialisedValue = handleInit(Bindings::Global, value, stack, bindings);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " global init '" << value.string() << "' to " << intialisedValue << '\n';
				}
			}
			break;
		case Instruction::ASSIGN_GLOBAL:
			{
				const Value &assignedValue = handleAssign(Bindings::Global, value, stack, bindings);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " global assign '" << value.string() << "' to " << assignedValue << '\n';
				}
			}
			break;
		case Instruction::REF_CLOSURE:
			handleRef(Bindings::Closure, value, stack, bindings);
			if(settings_.trace)
			{
				std::cout << "DEBUG: " << it->sourceLocation() << " closure ref '" << value.string() << "' is " << stack.back() << '\n';
			}
			break;
		case Instruction::INIT_CLOSURE:
			{
				Identifier identifier = Identifier(value.string());
				Bindings::ValuePtr &binding = bindings.getPointer(identifier);
				closureValues.push_back(ClosedNameAndValue(identifier, binding));
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " closure init '" << value.string() << "' is " << *binding << '\n';
				}
			}
			break;
		case Instruction::ASSIGN_CLOSURE:
			{
				const Value &assignedValue = handleAssign(Bindings::Closure, value, stack, bindings);
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " closure assign '" << value.string() << "' to " << assignedValue << '\n';
				}
			}
			break;
		case Instruction::MEMBER_ACCESS:
			{
				if(settings_.trace)
				{
					std::cout << "DEBUG: " << it->sourceLocation() << " member access " << value << '\n';
				}
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

			if (closureValues.empty())
			{
				std::cout << "closureValues is empty\n";
			}
			else
			{
				std::cout << "closureValues contains " << closureValues.size() << " entries:\n";
				int index = 0;
				for(const ClosedNameAndValue &closedValue: closureValues)
				{
					++index;
					std::cout << index << ":  " << closedValue.first << " -> " << *closedValue.second << " @ " << closedValue.second << '\n';
				}
			}
		}
	}
	
	return stack.empty() ? Value::nil() : pop(stack);
}

Value Interpreter::handleFunction(const SourceLocation &sourceLocation, const Value &value, Stack &stack, Bindings &bindings)
{
	unsigned argc = getArgumentCount(value);
	if(stack.size() < argc + 1)
	{
		throw CompilerBug("Need " + str(argc + 1) + " values on stack to call function, but only have " + str(stack.size()));
	}

	Value top = pop(stack);
	if(!top.isFunction())
	{
		throw ExecutionError(sourceLocation, "Call instruction expects top of the stack to be functional value, but got: " + str(top));
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
		CallContext callContext(&bindings.globals(), arguments, this);
		return function.call(callContext);
	}
	catch (RaspError &error)
	{
		error.buildStackTrace(" at function: " + function.name(), function.sourceLocation());
		throw;
	}
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
	return (i == globals_.end() ? nullptr : i->second.get());
}

