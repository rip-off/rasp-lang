#include "standard_math.h"

#include "api.h"
#include "standard_library_error.h"

namespace
{
	#define CURRENT_SOURCE_LOCATION SourceLocation(__FILE__, __LINE__)
	#define ExternalFunctionError(message) ExternalFunctionError(__FUNCTION__, CURRENT_SOURCE_LOCATION, message)

	Value plus(const Arguments &arguments)
	{
		if(arguments.size() < 2)
		{
			throw ExternalFunctionError("Expected at least two arguments");
		}

		Arguments::const_iterator i = arguments.begin();
		if(i->isNumber())
		{
			int result = 0;
			for( /* i */ ; i != arguments.end() ; ++i)
			{
				if(!i->isNumber())
				{
					throw ExternalFunctionError("Expected numeric argument");
				}
				result += i->number();
			}
			return Value::number(result);
		}
 		else if(i->isString())
		{
			std::string result;
			for( /* i */ ; i != arguments.end() ; ++i)
			{
				if(!i->isString())
				{
					throw ExternalFunctionError("Expected string argument");
				}
				result += i->string();
			}
			return Value::string(result);
		}
		else
		{
			throw ExternalFunctionError("Expected string or integer arguments");
		}
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

	int sub(int x, int y)
	{
		return x - y;
	}

	int div(int x, int y)
	{
		// TODO: check for division by zero?
		return x / y;
	}

	int mod(int x, int y)
	{
		return x % y;
	}

	bool less(int x, int y)
	{
		return x < y;
	}

	bool greater(int x, int y)
	{
		return x > y;
	}

	bool lessEqual(int x, int y)
	{
		return x <= y;
	}

	bool greaterEqual(int x, int y)
	{
		return x >= y;
	}

	Value operatorNot(const Arguments &arguments)
	{
		if(arguments.size() != 1 || !arguments[0].isBoolean())
		{
			throw ExternalFunctionError("Expected 1 boolean argument");
		}
		return Value::boolean(!arguments[0].boolean());
	}

	Value operatorOr(const Arguments &arguments)
	{
		if(arguments.size() < 2)
		{
			throw ExternalFunctionError("Expected at least 2 arguments");
		}
		bool result = false;
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			if(!i->isBoolean())
			{
				throw ExternalFunctionError("Expected boolean argument");
			}
			if (i->boolean())
			{
				result = true;
			}
		}
		return Value::boolean(result);
	}

	Value operatorAnd(const Arguments &arguments)
	{
		if(arguments.size() < 2)
		{
			throw ExternalFunctionError("Expected at least 2 arguments");
		}
		bool result = true;
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			if(!i->isBoolean())
			{
				throw ExternalFunctionError("Expected boolean argument");
			}
			if (!i->boolean())
			{
				result = false;
			}
		}
		return Value::boolean(result);
	}

	Value equal(const Arguments &arguments)
	{
		if(arguments.size() != 2)
		{
			throw ExternalFunctionError("Expected 2 arguments");
		}
		return Value::boolean(arguments[0] == arguments[1]);
	}

	Value notEqual(const Arguments &arguments)
	{
		if(arguments.size() != 2)
		{
			throw ExternalFunctionError("Expected 2 arguments");
		}
		return Value::boolean(arguments[0] != arguments[1]);
	}

	void expectTwoNumbers(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
	}

	template<int (*function)(int, int)>
	Value numericOperation(const Arguments &arguments)
	{
		expectTwoNumbers(arguments);
		int result = function(arguments[0].number(), arguments[1].number());
		return Value::number(result);
	}

	template<bool (*predicate)(int, int)>
	Value numericPredicate(const Arguments &arguments)
	{
		expectTwoNumbers(arguments);
		bool result = predicate(arguments[0].number(), arguments[1].number());
		return Value::boolean(result);
	}

	const ApiReg registry[] = 
	{
		ApiReg("+", CURRENT_SOURCE_LOCATION, &plus),
		ApiReg("-", CURRENT_SOURCE_LOCATION, numericOperation<&sub>),
		ApiReg("/", CURRENT_SOURCE_LOCATION, numericOperation<&div>),
		ApiReg("*", CURRENT_SOURCE_LOCATION, &mul),
		ApiReg("%", CURRENT_SOURCE_LOCATION, numericOperation<&mod>),
		ApiReg("<", CURRENT_SOURCE_LOCATION, numericPredicate<&less>),
		ApiReg(">", CURRENT_SOURCE_LOCATION, numericPredicate<&greater>),
		ApiReg("<=", CURRENT_SOURCE_LOCATION, numericPredicate<&lessEqual>),
		ApiReg(">=", CURRENT_SOURCE_LOCATION, numericPredicate<&greaterEqual>),
		// TODO: move out of math?
		ApiReg("!", CURRENT_SOURCE_LOCATION, &operatorNot),
		ApiReg("==", CURRENT_SOURCE_LOCATION, &equal),
		ApiReg("!=", CURRENT_SOURCE_LOCATION, &notEqual),
		ApiReg("||", CURRENT_SOURCE_LOCATION, &operatorOr),
		ApiReg("&&", CURRENT_SOURCE_LOCATION, &operatorAnd),
	};
}

void standardMath(Bindings::Mapping &mappings)
{
	registerBindings(mappings, registry);
}

