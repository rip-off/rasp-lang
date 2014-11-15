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
		bool boolean;
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
		TBoolean,
		TFunction,
	};

	Value();

	// Rule of three
	~Value();
	Value(const Value &);
	Value &operator=(const Value &);
	friend void swap(Value &a, Value &b);

	static Value nil();
	static Value boolean(bool boolean);
	static Value number(int number);
	static Value string(const std::string &text);
	static Value function(const Function &function);

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

	bool isBoolean() const
	{
		return type_ == TBoolean;
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

	bool boolean() const
	{
		assert(isBoolean());
		return data_.boolean;
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

	bool asBool() const;

	friend std::ostream &operator<<(std::ostream &out, const Value &value);


	explicit Value(bool boolean);
	/* TODO: explicit */ Value(int number);
	/* TODO: explicit */ Value(const std::string &text);
	/* TODO: explicit */ Value(const Function &function);
private:

	Type type_;
	Data data_;
};

#endif
