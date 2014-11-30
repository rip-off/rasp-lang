#include "standard_library.h"

#include <ctime>
#include <iostream>

#include "api.h"
#include "exceptions.h"

namespace
{
	class ExternalFunctionError : public RaspError
	{
	public:
		ExternalFunctionError(const std::string &function, const std::string &message)
		:
			RaspError(message + " in external function '" + function + "'")
		{
		}
	};

	#define ExternalFunctionError(message) ExternalFunctionError(__FUNCTION__, message)

	Value plus(const Arguments &arguments)
	{
		int result = 0;
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			if(!i->isNumber())
			{
				throw ExternalFunctionError("Expected numeric argument");
			}
			result += i->number();
		}
		return Value::number(result);
	}

	Value mul(const Arguments &arguments)
	{
		int result = 1;
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			if(!i->isNumber())
			{
				throw ExternalFunctionError("Expected numeric argument");
			}
			result *= i->number();
		}
		return Value::number(result);
	}

	Value sub(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return Value::number(arguments[0].number() - arguments[1].number());
	}

	Value div(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return Value::number(arguments[0].number() / arguments[1].number());
	}

	// TODO: probably don't need this anymore
	Value handleBool(bool b)
	{
		return Value::boolean(b);
	}

	Value less(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return handleBool(arguments[0].number() < arguments[1].number());
	}

	Value greater(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return handleBool(arguments[0].number() > arguments[1].number());
	}

	Value operator_not(const Arguments &arguments)
	{
		if(arguments.size() != 1 || !arguments[0].isBoolean())
		{
			throw ExternalFunctionError("Expected 1 boolean argument");
		}
		return !arguments[0].boolean();
	}

	Value equal(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return handleBool(arguments[0].number() == arguments[1].number());
	}

	Value notEqual(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return handleBool(arguments[0].number() != arguments[1].number());
	}

	Value lessEqual(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return handleBool(arguments[0].number() <= arguments[1].number());
	}

	Value greaterEqual(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return handleBool(arguments[0].number() >= arguments[1].number());
	}

	Value print(const Arguments &arguments)
	{
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			const Value &value = *i;
			switch(value.type())
			{
			case Value::TNil:
				std::cout << "nil";
				break;
			case Value::TString:
				std::cout << value.string();
				break;
			case Value::TNumber:
				std::cout << value.number();
				break;
			case Value::TFunction:
				std::cout << value;
				break;
			default:
				throw std::logic_error("Type not implemented");
			}
		}
		return Value();
	}

	Value println(const Arguments &arguments)
	{
		Value result = print(arguments);
		std::cout << '\n';
		return result;
	}

	Value time(const Arguments &arguments)
	{
		if(!arguments.empty())
		{
			throw ExternalFunctionError("Expect no arguments");
		}
		return Value::number(std::time(0));
	}

#define ENTRY(X) ApiReg(#X, &X)

	const ApiReg registry[] = 
	{
		ApiReg("+", &plus),
		ApiReg("-", &sub),
		ApiReg("/", &div),
		ApiReg("*", &mul),
		ApiReg("<", &less),
		ApiReg(">", &greater),
		ApiReg("!", &operator_not),
		ApiReg("==", &equal),
		ApiReg("!=", &notEqual),
		ApiReg("<=", &lessEqual),
		ApiReg(">=", &greaterEqual),
		ENTRY(time),
		ENTRY(print),
		ENTRY(println),
	};

#undef ENTRY
}



Bindings bindStandardLibrary()
{
	Bindings result;
	registerBindings(result, registry);
	return result;
}
