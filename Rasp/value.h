#ifndef VALUE_H
#define VALUE_H

#include <iosfwd>
#include <cassert>

class Function;

class Value
{
private:
	enum Type
	{
		TNil,
		TNumber,
		TFunction,
	};

	union Data
	{
		int number;
		const Function *function;
	};

public:
	Value()
		: type(TNil)
	{
	}

	Value(int number)
		: type(TNumber)
	{
		data.number = number;
	}

	Value(const Function &function);

	// Rule of three
	~Value();
	Value(const Value &);
	Value &operator=(const Value &);

	static Value nil()
	{
		return Value();
	}
	
	bool isNumber() const 
	{ 
		return type == TNumber; 
	}

	bool isFunction() const 
	{ 
		return type == TFunction; 
	}

	int number() const
	{
		assert(isNumber());
		return data.number;
	}

	const Function &function() const
	{
		assert(isFunction());
		return *data.function;
	}

	friend std::ostream &operator<<(std::ostream &out, const Value &value);

private:
	Type type;
	Data data;
};

#endif
