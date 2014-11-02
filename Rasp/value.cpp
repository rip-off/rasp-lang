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

void swap(Value &a, Value &b)
{
	using std::swap;
	swap(a.type_, b.type_);
	swap(a.data_, b.data_);
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
	case Value::TFunction:
		return out << "(function: " << value.data_.function->name() << ')';
	default:
		throw std::logic_error("Type not implemented");
	}
}
