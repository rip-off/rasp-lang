#include "value.h"

#include <cstring>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "escape.h"
#include "function.h"

Value::Value()
	: type_(TNil)
{
}

Value::Value(int number)
	: type_(TNumber)
{
	data_.number = number;
}

Value::Value(bool boolean)
	: type_(TBoolean)
{
	data_.boolean = boolean;
}

Value::Value(const Function &function)
	: type_(TFunction)
{
	data_.function = function.clone();
}

Value::Value(const std::string &text)
	: type_(TString)
{
	data_.string = new std::string(text);
}

Value::~Value()
{
	if(type_ == TFunction)
	{
		delete data_.function;
	}
	else if(type_ == TString)
	{
		delete data_.string;
	}
}

Value::Value(const Value &value)
	: type_(value.type_)
{
	if(type_ == TFunction)
	{
		data_.function = value.data_.function->clone();
	}
	else if(type_ == TString)
	{
		data_.string = new std::string(*value.data_.string);
	}
	else
	{
		data_ = value.data_;
	}
}

Value &Value::operator=(const Value &value)
{
	Value copy = value;
	swap(copy, *this);
	return *this;
}

Value Value::nil()
{
	return Value();
}

Value Value::boolean(bool boolean)
{
	return Value(boolean);
}

Value Value::number(int number)
{
	return Value(number);
}

Value Value::string(const std::string &text)
{
	return Value(text);
}

Value Value::function(const Function &function)
{
	return Value(function);
}

void swap(Value &a, Value &b)
{
	using std::swap;
	swap(a.type_, b.type_);
	swap(a.data_, b.data_);
}

bool Value::asBool() const
{
	switch(type_)
	{
	case Value::TNil:
		return false;
	case Value::TString:
		return !data_.string->empty();
	case Value::TNumber:
		return data_.number != 0;
	case Value::TBoolean:
		return data_.boolean;
	case Value::TFunction:
		return true;
	default:
		throw std::logic_error("Type not implemented");
	}
}

namespace
{
	std::string addEscapes(const std::string &text)
	{
		std::string result;
		for (unsigned i = 0 ; i < text.size() ; ++i) 
		{
			char c = text[i];
			if (needsReescaping(c)) 
			{
				result += '\\';
				result += reescape(c);
			}
			else
			{
				result += c;
			}
		}
		return result;
	}
}

std::ostream &operator<<(std::ostream &out, const Value &value)
{
	switch(value.type_)
	{
	case Value::TNil:
		return out << "nil";
	case Value::TString:
		return out << '\"' << addEscapes(*value.data_.string) << '\"';
	case Value::TNumber:
		return out << value.data_.number;
	case Value::TBoolean:
		return out << (value.data_.boolean ? "true" : "false");
	case Value::TFunction:
		return out << "(function: " << value.data_.function->name() << ')';
	default:
		throw std::logic_error("Type not implemented");
	}
}
