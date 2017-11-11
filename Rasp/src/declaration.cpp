#include "declaration.h"

Declaration::Declaration(const Identifier &identifier)
:
	identifier_(identifier)
{
}

Declaration::Declaration(const Identifier &identifier, const std::string &type)
:
	identifier_(identifier),
	type_(type)
{
}

