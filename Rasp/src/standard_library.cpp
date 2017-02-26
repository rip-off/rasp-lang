#include "standard_library.h"

#include <ctime>
#include <iostream>

#include "api.h"
#include "exceptions.h"

namespace
{
	void print_to_stream(std::ostream &out, const Arguments &arguments)
	{
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			const Value &value = *i;
			switch(value.type())
			{
			case Value::TNil:
				out << "nil";
				break;
			case Value::TString:
				out << value.string();
				break;
			case Value::TNumber:
				out << value.number();
				break;
			case Value::TBoolean:
				out << (value.boolean() ? "true" : "false");
				break;
			default:
				// TODO: Handle such values better?
				out << value;
				break;
			}
		}
	}
}

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

	Value less(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return Value::boolean(arguments[0].number() < arguments[1].number());
	}

	Value greater(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return Value::boolean(arguments[0].number() > arguments[1].number());
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

	Value lessEqual(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return Value::boolean(arguments[0].number() <= arguments[1].number());
	}

	Value greaterEqual(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExternalFunctionError("Expected 2 numeric arguments");
		}
		return Value::boolean(arguments[0].number() >= arguments[1].number());
	}

	Value print(const Arguments &arguments)
	{
		print_to_stream(std::cout, arguments);
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

	Value array_length(const Arguments &arguments)
	{
		if(arguments.size() != 1 || !arguments[0].isArray())
		{
			throw ExternalFunctionError("Expected 1 array argument");
		}
		return Value::number(arguments[0].array().size());
	}

	Value array_element(const Arguments &arguments)
	{
		if(arguments.size() != 2)
		{
			throw ExternalFunctionError("Expected 2 arguments");
		}

		const Value &arrayValue = arguments[0];
		if(!arrayValue.isArray())
		{
			throw ExternalFunctionError("Expected array first argument");
		}
		const Value::Array &array = arrayValue.array();

		const Value &indexValue = arguments[1];
		if(!indexValue.isNumber())
		{
			throw ExternalFunctionError("Expected numeric argument");
		}

		int index = indexValue.number();
		if (index < 0 || index >= array.size())
		{
			throw ExternalFunctionError("Array has " + str(array.size()) + " elements, cannot get index " + str(index));
		}
		return array[index];
	}

	Value array_set_element(const Arguments &arguments)
	{
		if(arguments.size() != 3)
		{
			throw ExternalFunctionError("Expected 3 arguments");
		}

		const Value &arrayValue = arguments[0];
		if(!arrayValue.isArray())
		{
			throw ExternalFunctionError("Expected array first argument");
		}
		Value::Array array = arrayValue.array();

		const Value &indexValue = arguments[1];
		if(!indexValue.isNumber())
		{
			throw ExternalFunctionError("Expected numeric argument");
		}

		int index = indexValue.number();
		if (index < 0 || index >= array.size())
		{
			throw ExternalFunctionError("Array has " + str(array.size()) + " elements, cannot get index " + str(index));
		}
		
		array[index] = arguments[2];
		return Value::array(array);
	}

	Value array_new(const Arguments &arguments)
	{
		if(arguments.size() != 1)
		{
			throw ExternalFunctionError("Expect 1 argument");
		}

		if(!arguments[0].isNumber())
		{
			throw ExternalFunctionError("Expected numeric argument");
		}

		int size = arguments[0].number();
		Value::Array result;
		for (unsigned i = 0 ; i < size ; ++i)
		{
			result.push_back(Value::nil());
		}
		return Value::array(result);
	}

	Value read_line(const Arguments &arguments)
	{
		if(!arguments.empty())
		{
			throw ExternalFunctionError("Expect no arguments");
		}
		std::string line;
		if (!std::getline(std::cin, line)) 
		{
			throw ExternalFunctionError("I/O error reading line");
		}
		return Value::string(line);
	}

	Value try_convert_string_to_int(const Arguments &arguments)
	{
		if(arguments.size() != 1 || !arguments[0].isString())
		{
			throw ExternalFunctionError("Expected 1 string argument");
		}
		const std::string &text = arguments[0].string();
		std::stringstream stream(text);
		int i;
		if (stream >> i && stream.eof())
		{
			return Value::number(i);
		}
		return Value::nil();
	}

	Value is_nil(const Arguments &arguments)
	{
		if(arguments.size() != 1)
		{
			throw ExternalFunctionError("Expected 1 argument");
		}
		return Value::boolean(arguments[0].isNil());
	}

	Value format(const Arguments &arguments)
	{
		if(arguments.empty())
		{
			throw ExternalFunctionError("Expected at least 1 argument");
		}
		std::stringstream stream;
		print_to_stream(stream, arguments);
		return Value::string(stream.str());
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
		ApiReg("!", &operatorNot),
		ApiReg("==", &equal),
		ApiReg("!=", &notEqual),
		ApiReg("<=", &lessEqual),
		ApiReg(">=", &greaterEqual),
		ApiReg("||", &operatorOr),
		ApiReg("&&", &operatorAnd),
		ENTRY(time),
		ENTRY(print),
		ENTRY(is_nil),
		ENTRY(format),
		ENTRY(println),
		ENTRY(read_line),
		ENTRY(array_length),
		ENTRY(array_element),
		ENTRY(array_set_element),
		ENTRY(array_new),
		ENTRY(try_convert_string_to_int),
	};

#undef ENTRY
}



Bindings bindStandardLibrary()
{
	Bindings result;
	registerBindings(result, registry);
	return result;
}
