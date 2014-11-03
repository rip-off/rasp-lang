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
		String,
		Number,
		Condition,
		Identifier,
		Declaration,
	};

	static Token root(unsigned line)
	{
		return Token(line, Root, "__root");
	}

	static Token nil(unsigned line)
	{
		return Token(line, Nil, "__nil_literal");
	}

	static Token condition(unsigned line)
	{
		return Token(line, Condition, "__if");
	}

	static Token declaration(unsigned line)
	{
		return Token(line, Declaration, "__def");
	}

	static Token list(unsigned line)
	{
		return Token(line, List, "__list");
	}

	static Token string(unsigned line, const std::string &text)
	{
		return Token(line, String, text);
	}

	static Token number(unsigned line, const std::string &number)
	{
		assert(is<int>(number));
		return Token(line, Number, number); 
	}

	static Token identifier(unsigned line, const std::string &identifier)
	{
		return Token(line, Identifier, identifier);
	}

	unsigned line() const
	{
		return line_;
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
	Token(unsigned line, Type type, const std::string &string)
	:
		line_(line),
		type_(type),
		string_(string)
	{
	}

	bool isValidChild() const
	{
		return type_ != Root;
	}

	unsigned line_;
	Type type_;
	std::string string_;
	Children children_;
};

#endif
