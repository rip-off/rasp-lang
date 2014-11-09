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
		Loop,
		String,
		Number,
		Condition,
		Identifier,
		Assignment,
		VariableDeclaration,
		FunctionDeclaration,
	};

	static Token root(unsigned line);

	static Token nil(unsigned line);

	static Token condition(unsigned line);

	static Token variableDeclaration(unsigned line);

	static Token functionDeclaration(unsigned line);

	static Token assignment(unsigned line);

	static Token list(unsigned line);

	static Token loop(unsigned line);

	static Token string(unsigned line, const std::string &text);

	static Token number(unsigned line, const std::string &number);

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
