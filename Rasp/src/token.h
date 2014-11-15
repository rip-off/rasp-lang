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
		Boolean,
		Keyword,
		Identifier,
	};

	static Token nil(unsigned line);

	static Token root(unsigned line);

	static Token list(unsigned line);

	static Token string(unsigned line, const std::string &text);

	static Token number(unsigned line, const std::string &number);

	static Token boolean(unsigned line, const std::string &boolean);

	static Token keyword(unsigned line, const std::string &keyword);

	static Token identifier(unsigned line, const std::string &identifier);

	unsigned line() const;

	Type type() const;

	const std::string &string() const;

	const Children &children() const;

	void addChild(const Token &token);

private:
	Token(unsigned line, Type type, const std::string &string);

	bool isValidChild() const;

	unsigned line_;
	Type type_;
	std::string string_;
	Children children_;
};

#endif
