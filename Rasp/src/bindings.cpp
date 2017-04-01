#include "bindings.h"

#include <algorithm>
#include "bug.h"

void Scope::add(const Identifier &identifier)
{
	if(isDefined(identifier))
	{
		throw CompilerBug("Identifier is already defined " + identifier.name());
	}
	declarations.push_back(identifier);
}

bool Scope::isDefined(const Identifier &identifier) const
{
	return std::find(declarations.begin(), declarations.end(), identifier) != declarations.end();
}

Declarations::Declarations()
{
	innerToOuterScopes.push_back(Scope());
}

Declarations::Declarations(const Bindings &globalScope)
{
	innerToOuterScopes.push_back(Scope());

	for(Bindings::const_iterator it = globalScope.begin() ; it != globalScope.end() ; ++it)
	{
		innerToOuterScopes[0].add(it->first);
	}
}

Declarations Declarations::newScope()
{
	Declarations result = *this;
	result.innerToOuterScopes.insert(result.innerToOuterScopes.begin(), Scope());
	return result;
}

void Declarations::add(const Identifier &identifier)
{
	assert(!innerToOuterScopes.empty());
	innerToOuterScopes.front().add(identifier);
}

bool Declarations::isDefined(const Identifier &identifier) const
{
	return checkIdentifier(identifier) != IDENTIFIER_DEFINITION_UNDEFINED;
}

Declarations::IdentifierDefinition Declarations::checkIdentifier(const Identifier &identifier) const
{
	for(std::vector<Scope>::size_type i = 0 ; i < innerToOuterScopes.size() ; ++i)
	{
		if(innerToOuterScopes[i].isDefined(identifier))
		{
			if (i == 0)
			{
				return IDENTIFIER_DEFINITION_LOCAL;
			}
			else if (i == innerToOuterScopes.size() - 1)
			{
				return IDENTIFIER_DEFINITION_GLOBAL;
			}
			return IDENTIFIER_DEFINITION_CLOSURE;
		}
	}
	return IDENTIFIER_DEFINITION_UNDEFINED;
}

