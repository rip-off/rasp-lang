#include "standard_library.h"

#include <ctime>
#include <iostream>

#include "api.h"
#include "standard_library_error.h"
#include "type_definition.h"

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
				// TODO: Should printing arbitrary types (functions, user defined types, ...) be allowed?
				out << value;
				break;
			}
		}
	}
}

namespace
{
	#define CURRENT_SOURCE_LOCATION SourceLocation(__FILE__, __LINE__)
	#define ExternalFunctionError(message) ExternalFunctionError(__FUNCTION__, CURRENT_SOURCE_LOCATION, message)

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

	Value concat(const Arguments &arguments)
	{
		if(arguments.size() < 2)
		{
			throw ExternalFunctionError("Expect at least 2 arguments");
		}
		std::stringstream result;
		print_to_stream(result, arguments);
		return Value::string(result.str());
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

	Value rasp_assert(const Arguments &arguments)
	{
		if(arguments.empty())
		{
			throw ExternalFunctionError("Too few arguments");
		}
		if(arguments.size() > 3)
		{
			throw ExternalFunctionError("Too few arguments");
		}

		if(arguments[0].isFalsey())
		{
			if (arguments.size() == 1)
			{
				throw ExternalFunctionError("assertion failed");
			}
			std::stringstream stream;
			stream << arguments[1];
			throw ExternalFunctionError("assertion failed: " + stream.str());
		}
		return Value::nil();
	}

	Value rasp_new(const Arguments &arguments)
	{
		if(arguments.empty())
		{
			throw ExternalFunctionError("Too few arguments");
		}
		if(!arguments.front().isTypeDefinition())
		{
			throw ExternalFunctionError("Expected first argument to be a type definition, but got " + str(arguments.front()));
		}

		const TypeDefinition &typeDefinition = arguments.front().typeDefinition();
		size_t memberCount = typeDefinition.memberNames.size();
		size_t constructorArguments = arguments.size() - 1;
		if (memberCount != constructorArguments)
		{
			throw ExternalFunctionError("Type " + typeDefinition.name + " requires " + str(memberCount) + " members, but found " + str(constructorArguments));
		}

		Value::Object object;
		for (size_t i = 0 ; i < memberCount ; ++i)
		{
			const std::string &memberName = typeDefinition.memberNames[i].name();
			object[memberName] = arguments[i + 1];
		}

		return Value::object(object);
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

#define ENTRY(X) ApiReg(#X, CURRENT_SOURCE_LOCATION, &X)

	const ApiReg registry[] = 
	{
		ApiReg("new", CURRENT_SOURCE_LOCATION, &rasp_new),
		ApiReg("assert", CURRENT_SOURCE_LOCATION, &rasp_assert),
		ApiReg("!", CURRENT_SOURCE_LOCATION, &operatorNot),
		ApiReg("==", CURRENT_SOURCE_LOCATION, &equal),
		ApiReg("!=", CURRENT_SOURCE_LOCATION, &notEqual),
		ApiReg("||", CURRENT_SOURCE_LOCATION, &operatorOr),
		ApiReg("&&", CURRENT_SOURCE_LOCATION, &operatorAnd),
		ENTRY(time),
		ENTRY(print),
		ENTRY(concat),
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

void standardLibrary(Bindings::Mapping &mappings)
{
	registerBindings(mappings, registry);
}

