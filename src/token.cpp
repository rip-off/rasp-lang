#include "token.h"
#include "bug.h"

Token Token::list(const SourceLocation &sourceLocation)
{
	return Token(sourceLocation, LIST, "__list");
}

Token Token::string(const SourceLocation &sourceLocation, const std::string &text)
{
	return Token(sourceLocation, STRING, text);
}

Token Token::number(const SourceLocation &sourceLocation, const std::string &number)
{
	assert(is<int>(number));
	return Token(sourceLocation, NUMBER, number); 
}

Token Token::keyword(const SourceLocation &sourceLocation, const std::string &keyword)
{
	return Token(sourceLocation, KEYWORD, keyword);
}

Token Token::identifier(const SourceLocation &sourceLocation, const Identifier &identifier)
{
	return Token(sourceLocation, IDENTIFIER, identifier.name());
}

Token Token::declaration(const SourceLocation &sourceLocation, const Declaration &declaration)
{
	const std::string &declarationType = declaration.type();
	if (declarationType.empty())
	{
		return identifier(sourceLocation, declaration.identifier());
	}
	Token result = Token(sourceLocation, DECLARATION, "__declaration");
	result.addChild(identifier(sourceLocation, declaration.identifier()));
	result.addChild(identifier(sourceLocation, Identifier(declarationType)));
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
	assert(!(type_ == STRING || type_ == NUMBER || type_ == KEYWORD));
	children_.push_back(token);
}


Token::Token(const SourceLocation &sourceLocation, Type type, const std::string &string)
:
	type_(type),
	string_(string),
	sourceLocation_(sourceLocation)
{
}

std::ostream &operator<<(std::ostream &out, Token::Type tokenType) {
	switch(tokenType) {
		case Token::LIST:
			return out << "List";
		case Token::STRING:
			return out << "String";
		case Token::NUMBER:
			return out << "Number";
		case Token::KEYWORD:
			return out << "Keyword";
		case Token::IDENTIFIER:
			return out << "Identifier";
		case Token::DECLARATION:
			return out << "Declaration";
		default:
			throw std::logic_error("Unhandled token type " + str(tokenType));
	}
}

