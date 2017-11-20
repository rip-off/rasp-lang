#ifndef VALUE_H
#define VALUE_H

#include <map>
#include <iosfwd>
#include <string>
#include <vector>
#include <cassert>

class Function;
struct TypeDefinition;

class Value
{
public:
	typedef std::vector<Value> Array;
	typedef std::map<std::string, Value> Object;
private:
	union Data
	{
		bool boolean;
		int number;
		Function *function;
		std::string *string;
		Array *array;
		Object *object;
		TypeDefinition *typeDefinition;
	};

public:
	enum Type
	{
		TNil,
		TArray,
		TString,
		TNumber,
		TObject,
		TBoolean,
		TFunction,
		TTypeDefinition,
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
	static Value object(const Object &object);
	static Value string(const std::string &text);
	static Value function(const Function &function);
	static Value typeDefinition(const TypeDefinition &typeDefinition);

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

	bool isObject() const
	{
		return type_ == TObject;
	}

	bool isBoolean() const
	{
		return type_ == TBoolean;
	}

	bool isFunction() const 
	{ 
		return type_ == TFunction; 
	}

	bool isTypeDefinition() const
	{
		return type_ == TTypeDefinition;
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

	const Object &object() const
	{
		assert(isObject());
		return *data_.object;
	}

	const std::string &string() const
	{
		assert(isString());
		return *data_.string;
	}

	const Function &function() const
	{
		assert(isFunction());
		return *data_.function;
	}

	const TypeDefinition &typeDefinition() const
	{
		assert(isTypeDefinition());
		return *data_.typeDefinition;
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

	bool isTruthy() const;
  bool isFalsey() const
  {
    return !isTruthy();
  }

	friend std::ostream &operator<<(std::ostream &out, const Value &value);
	friend bool operator==(const Value &left, const Value &right);
	friend bool operator!=(const Value &left, const Value &right)
	{
		bool equal = (left == right);
		return !equal;
	}

private:
	explicit Value(bool boolean);
	explicit Value(int number);
	explicit Value(const std::string &text);
	explicit Value(const Function &function);
	explicit Value(const Array &array);
	explicit Value(const Object &object);
	explicit Value(const TypeDefinition &typeDefinition);

	Type type_;
	Data data_;
};

#endif
