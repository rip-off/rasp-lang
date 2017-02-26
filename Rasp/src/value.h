#ifndef VALUE_H
#define VALUE_H

#include <iosfwd>
#include <string>
#include <vector>
#include <cassert>

class Function;

class Value
{
public:
	typedef std::vector<Value> Array;
private:
	union Data
	{
		bool boolean;
		int number;
		Function *function;
		std::string *string;
		Array *array;
	};

public:
	enum Type
	{
		TNil,
		TArray,
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
	static Value array(const Array &elements);
	static Value boolean(bool boolean);
	static Value number(int number);
	static Value string(const std::string &text);
	static Value function(const Function &function);

	bool isNil() const
	{
		return type_ == TNil;
	}
	
	bool isArray() const
	{
		return type_ == TArray;
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

	const Array &array() const
	{
		assert(isArray());
		return *data_.array;	
	}

	bool asBool() const;

	friend std::ostream &operator<<(std::ostream &out, const Value &value);
	friend bool operator==(const Value &left, const Value &right);
	friend bool operator!=(const Value &left, const Value &right)
	{
		bool equal = (left == right);
		return !equal;
	}


	explicit Value(bool boolean);
	explicit Value(int number);
	explicit Value(const std::string &text);
	/* TODO: explicit */ Value(const Function &function);
private:
	explicit Value(const Array &array);

	Type type_;
	Data data_;
};

#endif
