#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <cassert>

#include "utils.h"

class Token
{
public:
	typedef std::vector<Token> Children;

	enum Type
	{
		Nil,
		Root,
		List,
		Number,
		Identifier
	};

	Token()
	: 
		type_(Root)
	{
	}

	static Token nil()
	{
		return Token(Nil, "__nil_literal");
	}

	static Token list()
	{
		return Token(List, "__list");
	}

	static Token number(const std::string &number)
	{
		assert(is<int>(number));
		return Token(Number, number); 
	}

	static Token identifier(const std::string &identifier)
	{
		return Token(Identifier, identifier);
	}

	Type type() const
	{
		return type_;
	}

	const std::string &string() const
	{
		return string_;
	}

	const Children &children() const
	{
		return children_;
	}

	void addChild(const Token &token)
	{
		if(token.isValidChild())
		{
			children_.push_back(token);
		}
	}

private:
	Token(Type type, const std::string &string)
	:
		type_(type),
		string_(string)
	{
	}

	bool isValidChild() const
	{
		return type_ != Root;
	}

	Type type_;
	std::string string_;
	Children children_;
};

#endif
