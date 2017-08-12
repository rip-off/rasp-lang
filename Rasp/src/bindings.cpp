#include "bindings.h"

#include <algorithm>
#include "bug.h"

Bindings::const_iterator Bindings::begin() const
{
    return valuesByName.begin();
}

Bindings::const_iterator Bindings::end() const
{
    return valuesByName.end();
}

Bindings::const_iterator Bindings::find(const Identifier &name) const
{
    return valuesByName.find(name);
}

Value &Bindings::operator[](const Identifier &name)
{
    return valuesByName[name];
}

    

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

IdentifierDefinition Declarations::checkIdentifier(const Identifier &identifier) const
{
	for(std::vector<Scope>::size_type i = 0 ; i < innerToOuterScopes.size() ; ++i)
	{
		if(innerToOuterScopes[i].isDefined(identifier))
		{
			if (i == innerToOuterScopes.size() - 1)
			{
				return IDENTIFIER_DEFINITION_GLOBAL;
			}
			else if (i == 0)
			{
				return IDENTIFIER_DEFINITION_LOCAL;
			}
			return IDENTIFIER_DEFINITION_CLOSURE;
		}
	}
	return IDENTIFIER_DEFINITION_UNDEFINED;
}

