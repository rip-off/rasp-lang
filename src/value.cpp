#include "value.h"

#include <cstring>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "bug.h"
#include "utils.h"
#include "escape.h"
#include "function.h"
#include "type_definition.h"
#include "execution_error.h"

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

Value::Value(const Object &object)
	: type_(TObject)
{
	data_.object = new Object(object);
}

Value::Value(const std::string &text)
	: type_(TString)
{
	data_.string = new std::string(text);
}

Value::Value(const Array &elements)
	: type_(TArray)
{
	data_.array = new Array(elements);
}

Value::Value(const TypeDefinition &typeDefinition)
	: type_(TTypeDefinition)
{
	data_.typeDefinition = new TypeDefinition(typeDefinition);
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
	else if(type_ == TObject)
	{
		delete data_.object;
	}
	else if(type_ == TArray)
	{
		delete data_.array;
	}
	else if(type_ == TTypeDefinition)
	{
		delete data_.typeDefinition;
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
	else if(type_ == TObject)
	{
		data_.object = new Object(*value.data_.object);
	}
	else if(type_ == TArray)
	{
		data_.array = new Array(*value.data_.array);
	}
	else if(type_ == TTypeDefinition)
	{
		data_.typeDefinition = new TypeDefinition(*value.data_.typeDefinition);
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

Value Value::array(const Array &array)
{
	return Value(array);
}

Value Value::boolean(bool boolean)
{
	return Value(boolean);
}

Value Value::number(int number)
{
	return Value(number);
}

Value Value::object(const Object &object)
{
	return Value(object);
}

Value Value::string(const std::string &text)
{
	return Value(text);
}

Value Value::function(const Function &function)
{
	return Value(function);
}

Value Value::typeDefinition(const TypeDefinition &typeDefinition)
{
	return Value(typeDefinition);
}

void swap(Value &a, Value &b)
{
	using std::swap;
	swap(a.type_, b.type_);
	swap(a.data_, b.data_);
}

bool Value::isTruthy() const
{
	switch(type_)
	{
	case Value::TNil:
		return false;
	case Value::TString:
		return !data_.string->empty();
	case Value::TNumber:
		return data_.number != 0;
	case Value::TObject:
		return true;
	case Value::TBoolean:
		return data_.boolean;
	case Value::TFunction:
		return true;
	case Value::TArray:
		return !data_.array->empty();
	case Value::TTypeDefinition:
		return true;
	default:
		throw CompilerBug("Type not implemented");
	}
}

std::ostream &operator<<(std::ostream &out, const Value::Type &type)
{
	switch(type)
	{
	case Value::TNil:
		return out << "TNil";
	case Value::TArray:
		return out << "TArray";
	case Value::TString:
		return out << "TString";
	case Value::TNumber:
		return out << "TNumber";
	case Value::TObject:
		return out << "TObject";
	case Value::TBoolean:
		return out << "TBoolean";
	case Value::TFunction:
		return out << "TFunction";
	case Value::TTypeDefinition:
		return out << "TTypeDefinition";
	default:
		throw CompilerBug("Type " + str(static_cast<int>(type)) + " not implemented");
	}
}

std::ostream &operator<<(std::ostream &out, const Value &value)
{
	switch(value.type_)
	{
	case Value::TNil:
		return out << "nil";
	case Value::TArray:
		{
			out << '[';
			const Value::Array &array = *value.data_.array;
			for (unsigned i = 0 ; i < array.size() ; ++i)
			{
				if (i > 0)
				{
					out << ", ";
				}
				out << array[i];
			}
			out << ']';
		}
		return out;
	case Value::TString:
		return out << '\"' << addEscapes(*value.data_.string) << '\"';
	case Value::TNumber:
		return out << value.data_.number;
	case Value::TObject:
		{
			out << '{';
			const Value::Object &object = *value.data_.object;
			for (Value::Object::const_iterator it = object.begin() ; it != object.end() ; ++it)
			{
				if (it != object.begin())
				{
					out << ", ";
				}
				out << it->first << " = " << it->second;
			}
			out << '}';
		}
		return out;
	case Value::TBoolean:
		return out << (value.data_.boolean ? "true" : "false");
	case Value::TFunction:
		return out << "<function: " << value.data_.function->name() << '>';
	case Value::TTypeDefinition:
		return out << "<type: " << value.data_.typeDefinition->name << '>';
	default:
		throw CompilerBug("Type " + str(value.type_) + " not implemented");
	}
}

namespace {
	bool arraysEqual(const Value::Array &leftArray, const Value::Array &rightArray)
	{
		if (leftArray.size() != rightArray.size())
		{
			return false;
		}

		for (unsigned i = 0 ; i < leftArray.size() ; ++i)
		{
			if (leftArray[i] != rightArray[i])
			{
				return false;
			}
		}
		return true;
	}

	bool objectsEquals(const Value::Object &leftObject, const Value::Object &rightObject)
	{
		if (leftObject.size() != rightObject.size())
		{
			return false;
		}

		for (Value::Object::const_iterator it = leftObject.begin() ; it != leftObject.end() ; ++it)
		{
			Value::Object::const_iterator rightElement = rightObject.find(it->first);
			if (rightElement == rightObject.end())
			{
				return false;
			}
			if (it->second != rightElement->second)
			{
				return false;
			}
		}
		return true;
	}
}

bool operator==(const Value &left, const Value &right)
{
	if (left.type_ != right.type_)
	{
		return false;
	}
	switch(left.type_)
	{
	case Value::TNil:
		return true;
	case Value::TArray:
		return arraysEqual(*left.data_.array, *right.data_.array);
	case Value::TString:
		return *left.data_.string == *right.data_.string;
	case Value::TNumber:
		return left.data_.number == right.data_.number;
	case Value::TObject:
		return objectsEquals(*left.data_.object, *right.data_.object);
	case Value::TBoolean:
		return left.data_.boolean == right.data_.boolean;
	case Value::TFunction:
		throw ExecutionError(CURRENT_SOURCE_LOCATION, "Comparing functions is not supported");
	case Value::TTypeDefinition:
		throw ExecutionError(CURRENT_SOURCE_LOCATION, "Comparing types is not supported");
	default:
		throw CompilerBug("Type " + str(left.type_) + " not implemented");
	}
}

