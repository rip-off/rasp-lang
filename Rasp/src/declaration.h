#ifndef DECLARATION_H
#define DECLARATION_H

#include "identifier.h"

class Declaration
{
public:
	explicit Declaration(const Identifier &identifier);
	explicit Declaration(const Identifier &identifier, const std::string &type);

	const Identifier &identifier() const
	{
		return identifier_;
	}

	const std::string &type() const
	{
		return type_;
	}

private:
	Identifier identifier_;
	std::string type_;
};

#endif

