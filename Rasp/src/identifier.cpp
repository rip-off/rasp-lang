#include "identifier.h"

#include "utils.h"
#include "bug.h"

Identifier::Identifier(const std::string &name) : name_(name)
{
	if(!isValid(name))
	{
		throw CompilerBug("Illegal attempt to construct an invalid identifier '" + name + "'");
	}
}

bool Identifier::isValid(const std::string &name)
{
	static const std::string whitelist[] =
	{
		"+",
		"-",
		"/",
		"*",
		"<",
		">",
		"==",
		"!=",
		"<=",
		">=",
	};
	if (array_is_element(whitelist, name))
	{
		return true;
	}

	if (name.empty())
	{
		return false;
	}

	if(!std::isalpha(name.front()))
	{
		return false;
	}

	for (unsigned i = 1 ; i < name.size() ; ++i)
	{
		char c = name[i];
		bool valid = std::isalnum(c) || (c == '_');
		if(!valid)
		{
			return false;
		}
	}
	return true;
}

