#include "value.h"

#include <cstring>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "function.h"

Value::Value(const Function &function)
	: type(TFunction)
{
	data.function = new Function(function);
}

Value::Value(const std::string &text)
	: type(TString)
{
	// TODO: strdup isn't standard???
	data.string = strdup(text.c_str());
}

Value::~Value()
{
	if(type == TFunction)
	{
		delete data.function;
	}
	else if(type == TString)
	{
		std::free(data.string);
	}
}

Value::Value(const Value &value)
	: type(value.type)
{
	if(type == TFunction)
	{
		data.function = new Function(*value.data.function);
	}
	else if(type == TString)
	{
		data.string = strdup(value.data.string);
	}
	else
	{
		data = value.data;
	}
}

Value &Value::operator=(const Value &value)
{
	Value copy = value;
	std::swap(copy, *this);
	return *this;
}

namespace
{
	std::string escapeEmbeddedQuotes(const std::string &text)
	{
		std::string result;
		for (unsigned i = 0 ; i < text.size() ; ++i) 
		{
			char c = text[i];
			if (c == '\"') 
			{
				result += '\\';
			}
			result += c;
		}
		return result;
	}
}

std::ostream &operator<<(std::ostream &out, const Value &value)
{
	switch(value.type)
	{
	case Value::TNil:
		return out << "nil";
	case Value::TString:
		// TODO: escape embedded quotes
		return out << "\"" << escapeEmbeddedQuotes(value.data.string) << "\"";
	case Value::TNumber:
		return out << value.data.number;
	case Value::TFunction:
		return out << "(function: " << value.data.function->name() << ')';
	default:
		throw std::logic_error("Type not implemented");
	}
}
