#include "standard_math.h"

#include "api.h"
#include "standard_library_error.h"

namespace
{
	#define CURRENT_SOURCE_LOCATION SourceLocation(__FILE__, __LINE__)
	#define ExternalFunctionError(message) ExternalFunctionError(__FUNCTION__, CURRENT_SOURCE_LOCATION, message)

	int plus(int x, int y)
	{
		return x + y;
	}

	int mul(int x, int y)
	{
		return x * y;
	}

	int sub(int x, int y)
	{
		return x - y;
	}

	int div(int x, int y)
	{
		if (y == 0) {
			throw ExternalFunctionError("cannot divide by zero");
		}
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

	void expectTwoNumbers(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
	}

	// TODO: ExternalFunctionError error messages mention 'numericFold'
	template<int init, int (*aggregate)(int, int)>
	Value numericFold(const Arguments &arguments)
	{
		if(arguments.size() < 2)
		{
			throw ExternalFunctionError("Expected at least 2 arguments");
		}
		int result = init;
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			if(!i->isNumber())
			{
				throw ExternalFunctionError("Expected numeric argument");
			}
			result = aggregate(result, i->number());
		}
		return Value::number(result);
	}

	template<int (*function)(int, int)>
	Value binaryOperation(const Arguments &arguments)
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
		ApiReg("+", CURRENT_SOURCE_LOCATION, numericFold<0, &plus>),
		ApiReg("*", CURRENT_SOURCE_LOCATION, numericFold<1, &mul>),
		ApiReg("-", CURRENT_SOURCE_LOCATION, binaryOperation<&sub>),
		ApiReg("/", CURRENT_SOURCE_LOCATION, binaryOperation<&div>),
		ApiReg("%", CURRENT_SOURCE_LOCATION, binaryOperation<&mod>),
		ApiReg("<", CURRENT_SOURCE_LOCATION, numericPredicate<&less>),
		ApiReg(">", CURRENT_SOURCE_LOCATION, numericPredicate<&greater>),
		ApiReg("<=", CURRENT_SOURCE_LOCATION, numericPredicate<&lessEqual>),
		ApiReg(">=", CURRENT_SOURCE_LOCATION, numericPredicate<&greaterEqual>),
	};
}

void standardMath(Bindings::Mapping &mappings)
{
	registerBindings(mappings, registry);
}

