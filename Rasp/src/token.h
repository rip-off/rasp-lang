#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <cassert>

#include "utils.h"
#include "declaration.h"
#include "source_location.h"

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
		Boolean,
		Keyword,
		Identifier,
		Declaration,
	};

	static Token nil(const SourceLocation &sourceLocation);

	static Token root(const SourceLocation &sourceLocation);

	static Token list(const SourceLocation &sourceLocation);

	static Token string(const SourceLocation &sourceLocation, const std::string &text);

	static Token number(const SourceLocation &sourceLocation, const std::string &number);

	static Token boolean(const SourceLocation &sourceLocation, const std::string &boolean);

	static Token keyword(const SourceLocation &sourceLocation, const std::string &keyword);

	// TODO: Identifier?
	static Token identifier(const SourceLocation &sourceLocation, const std::string &identifier);

	static Token declaration(const SourceLocation &sourceLocation, const ::Declaration &declaration);

	const SourceLocation &sourceLocation() const;

	Type type() const;

	const std::string &string() const;

	const Children &children() const;

	void addChild(const Token &token);

private:
	Token(const SourceLocation &sourceLocation, Type type, const std::string &string);

	bool isValidChild() const;

	Type type_;
	std::string string_;
	Children children_;
	SourceLocation sourceLocation_;
};

std::ostream &operator<<(std::ostream &out, Token::Type tokenType);

#endif
