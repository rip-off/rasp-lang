#include "standard_math.h"

#include "api.h"
#include "standard_library_error.h"

namespace
{
	#define CURRENT_SOURCE_LOCATION SourceLocation(__FILE__, __LINE__)
	#define ExternalFunctionError(functionName, message) ExternalFunctionError(functionName, CURRENT_SOURCE_LOCATION, message)

	enum MathFunction {
		ADD,
		SUB,
		MUL,
		DIV,
		MOD,
		LT,
		GT,
		LTE,
		GTE,
	};

	const char *functionName(MathFunction function)
	{
		switch(function) {
		case ADD: return "+";
		case SUB: return "-";
		case MUL: return "*";
		case DIV: return "/";
		case MOD: return "%";
		case LT:  return "<";
		case GT:  return ">";
		case LTE: return "<=";
		case GTE: return ">=";
		}
	}

	int add(int x, int y)
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
			throw ExternalFunctionError(functionName(DIV), "cannot divide by zero");
		}
		return x / y;
	}

	int mod(int x, int y)
	{
		// TODO: check for mod by zero
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

	void expectTwoNumbers(MathFunction mathFunction, const Arguments &arguments)
	{
		if(arguments.size() != 2)
		{
			throw ExternalFunctionError(functionName(mathFunction), "Expected 2 arguments");
		}

		if(!(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError(functionName(mathFunction), "Expected numeric argument");
		}
	}

	template<MathFunction math, int init, int (*aggregate)(int, int)>
	Value numericFold(const Arguments &arguments)
	{
		if(arguments.size() < 2)
		{
			throw ExternalFunctionError(functionName(math), "Expected at least 2 arguments");
		}
		int result = init;
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			if(!i->isNumber())
			{
				throw ExternalFunctionError(functionName(math), "Expected numeric argument");
			}
			result = aggregate(result, i->number());
		}
		return Value::number(result);
	}

	template<MathFunction math, int (*function)(int, int)>
	Value binaryOperation(const Arguments &arguments)
	{
		expectTwoNumbers(math, arguments);
		int result = function(arguments[0].number(), arguments[1].number());
		return Value::number(result);
	}

	template<MathFunction math, bool (*predicate)(int, int)>
	Value numericPredicate(const Arguments &arguments)
	{
		expectTwoNumbers(math, arguments);
		bool result = predicate(arguments[0].number(), arguments[1].number());
		return Value::boolean(result);
	}

	const ApiReg registry[] = 
	{
		ApiReg(functionName(ADD), CURRENT_SOURCE_LOCATION, numericFold<ADD, 0, &add>),
		ApiReg(functionName(MUL), CURRENT_SOURCE_LOCATION, numericFold<MUL, 1, &mul>),
		ApiReg(functionName(SUB), CURRENT_SOURCE_LOCATION, binaryOperation<SUB, &sub>),
		ApiReg(functionName(DIV), CURRENT_SOURCE_LOCATION, binaryOperation<DIV, &div>),
		ApiReg(functionName(MOD), CURRENT_SOURCE_LOCATION, binaryOperation<MOD, &mod>),
		ApiReg(functionName(LT),  CURRENT_SOURCE_LOCATION, numericPredicate<LT, &less>),
		ApiReg(functionName(GT),  CURRENT_SOURCE_LOCATION, numericPredicate<GT, &greater>),
		ApiReg(functionName(LTE), CURRENT_SOURCE_LOCATION, numericPredicate<LTE, &lessEqual>),
		ApiReg(functionName(GTE), CURRENT_SOURCE_LOCATION, numericPredicate<GTE, &greaterEqual>),
	};
}

void standardMath(Bindings::Mapping &mappings)
{
	registerBindings(mappings, registry);
}

