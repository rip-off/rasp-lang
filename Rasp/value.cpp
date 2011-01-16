#include "value.h"

#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "function.h"

Value::Value(const Function &function)
	: type(TFunction)
{
	data.function = new Function(function);
}

Value::~Value()
{
	if(type == TFunction)
	{
		delete data.function;
	}
}

Value::Value(const Value &value)
	: type(value.type)
{
	if(type == TFunction)
	{
		data.function = new Function(*value.data.function);
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

std::ostream &operator<<(std::ostream &out, const Value &value)
{
	switch(value.type)
	{
	case Value::TNil:
		return out << "nil";
	case Value::TNumber:
		return out << value.data.number;
	case Value::TFunction:
		return out << "(function: " << value.data.function->name() << ')';
	default:
		throw std::logic_error("Type not implemented");
	}
}
