#ifndef VALUE_H
#define VALUE_H

#include <iosfwd>
#include <string>
#include <cassert>

class Function;

class Value
{
	union Data
	{
		int number;
		Function *function;
		std::string *string;
	};

public:
	enum Type
	{
		TNil,
		TString,
		TNumber,
		TFunction,
	};

	Value();
	Value(int number);
	Value(const std::string &text);
	Value(const Function &function);

	// Rule of three
	~Value();
	Value(const Value &);
	Value &operator=(const Value &);
	friend void swap(Value &a, Value &b);

	static Value nil()
	{
		return Value();
	}

	bool isNil() const
	{
		return type_ == TNil;
	}
	
	bool isNumber() const 
	{ 
		return type_ == TNumber; 
	}

	bool isString() const
	{
		return type_ == TString;
	}

	bool isFunction() const 
	{ 
		return type_ == TFunction; 
	}

	int number() const
	{
		assert(isNumber());
		return data_.number;
	}

	std::string string() const
	{
		assert(isString());
		return *data_.string;
	}

	const Function &function() const
	{
		assert(isFunction());
		return *data_.function;
	}

	Type type() const
	{
		return type_;
	}

	friend std::ostream &operator<<(std::ostream &out, const Value &value);

private:
	Type type_;
	Data data_;
};

#endif
