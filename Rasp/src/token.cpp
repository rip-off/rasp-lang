#include "token.h"
#include "bug.h"

Token Token::nil(const SourceLocation &sourceLocation)
{
	return Token(sourceLocation, Nil, "__nil_literal");
}

Token Token::root(const SourceLocation &sourceLocation)
{
	return Token(sourceLocation, Root, "__root");
}

Token Token::list(const SourceLocation &sourceLocation)
{
	return Token(sourceLocation, List, "__list");
}

Token Token::string(const SourceLocation &sourceLocation, const std::string &text)
{
	return Token(sourceLocation, String, text);
}

Token Token::number(const SourceLocation &sourceLocation, const std::string &number)
{
	assert(is<int>(number));
	return Token(sourceLocation, Number, number); 
}

Token Token::keyword(const SourceLocation &sourceLocation, const std::string &keyword)
{
	return Token(sourceLocation, Keyword, keyword);
}

Token Token::boolean(const SourceLocation &sourceLocation, const std::string &boolean)
{
	return Token(sourceLocation, Boolean, boolean);
}

Token Token::identifier(const SourceLocation &sourceLocation, const std::string &identifier)
{
	return Token(sourceLocation, Identifier, identifier);
}

Token Token::declaration(const SourceLocation &sourceLocation, const ::Declaration &declaration)
{
	const std::string &type = declaration.type();
	if (type.empty())
	{
		return identifier(sourceLocation, declaration.identifier().name());
	}
	Token result = Token(sourceLocation, Declaration, "__declaration");
	result.addChild(identifier(sourceLocation, declaration.identifier().name()));
	result.addChild(identifier(sourceLocation, type));
	return result;
}

const SourceLocation &Token::sourceLocation() const
{
	return sourceLocation_;
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


Token::Token(const SourceLocation &sourceLocation, Type type, const std::string &string)
:
	type_(type),
	string_(string),
	sourceLocation_(sourceLocation)
{
}

bool Token::isValidChild() const
{
	return type_ != Root;
}

std::ostream &operator<<(std::ostream &out, Token::Type tokenType) {
	switch(tokenType) {
		case Token::Nil:
			return out << "Nil";
		case Token::Root:
			return out << "Root";
		case Token::List:
			return out << "List";
		case Token::String:
			return out << "String";
		case Token::Number:
			return out << "Number";
		case Token::Boolean:
			return out << "Boolean";
		case Token::Keyword:
			return out << "Keyword";
		case Token::Identifier:
			return out << "Identifier";
		case Token::Declaration:
			return out << "Declaration";
	}
}

