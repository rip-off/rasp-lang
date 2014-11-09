#include "token.h"

Token Token::nil(unsigned line)
{
	return Token(line, Nil, "__nil_literal");
}

Token Token::root(unsigned line)
{
	return Token(line, Root, "__root");
}

Token Token::list(unsigned line)
{
	return Token(line, List, "__list");
}

Token Token::string(unsigned line, const std::string &text)
{
	return Token(line, String, text);
}

Token Token::number(unsigned line, const std::string &number)
{
	assert(is<int>(number));
	return Token(line, Number, number); 
}

Token Token::keyword(unsigned line, const std::string &keyword)
{
	return Token(line, Keyword, keyword);
}

Token Token::identifier(unsigned line, const std::string &identifier)
{
	return Token(line, Identifier, identifier);
}

unsigned Token::line() const
{
	return line_;
}

Token::Type Token::type() const
{
	return type_;
}

const std::string &Token::string() const
{
	return string_;
}

const Token::Children &Token::children() const
{
	return children_;
}

void Token::addChild(const Token &token)
{
	if(token.isValidChild())
	{
		children_.push_back(token);
	}
}


Token::Token(unsigned line, Type type, const std::string &string)
:
	line_(line),
	type_(type),
	string_(string)
{
}

bool Token::isValidChild() const
{
	return type_ != Root;
}

