#include "bindings.h"

#include <algorithm>
#include "bug.h"
#include "utils.h"

Bindings::Bindings(Mapping *globalsByName)
:
	globalsByName_(globalsByName),
	closedValuesByName_(nullptr)
{
}

Bindings::Bindings(Mapping *globalsByName, Mapping *closedValuesByName)
:
	globalsByName_(globalsByName),
	closedValuesByName_(closedValuesByName)
{
}

const Value &Bindings::get(RefType refType, const Identifier &identifier) const
{
	const Mapping &mapping = mappingFor(refType);
	Bindings::const_iterator it = mapping.find(identifier);
	if (it == mapping.end())
	{
		throw CompilerBug("Cannot get an unbound " + str(refType) + " identifier: '" + identifier.name() + "'");
	}
	return *it->second;
}

void Bindings::set(RefType refType, const Identifier &identifier, const Value &value)
{
	Mapping &mapping = mappingFor(refType);
	Bindings::const_iterator it = mapping.find(identifier);
	if (it == mapping.end())
	{
		throw CompilerBug("Cannot set an unbound " + str(refType) + " identifier: '" + identifier.name() + "'");
	}
	*mapping[identifier] = value;
}

void Bindings::init(RefType refType, const Identifier &identifier, const Value &value)
{
    Mapping &mapping = mappingFor(refType);
	#if 1
	mapping[identifier] = makeValue(value);
	#else // TODO: fix testLoopWithInnerVariableDeclaration
	auto result = mapping.insert(std::make_pair(identifier, value));
	if (!result.second)
	{
		throw CompilerBug("Cannot initialise an already bound " + str(refType) + " identifier: '" + identifier.name() + "'");
	}
	#endif
}

void Bindings::initLocal(const Identifier &identifier, const Value &value)
{
	auto result = localsByName_.insert(std::make_pair(identifier, makeValue(value)));
	if (!result.second)
	{
		throw CompilerBug("Cannot initialise an already bound local identifier: '" + identifier.name() + "'");
	}
}

Bindings::ValuePtr &Bindings::getPointer(const Identifier &identifier)
{
	Bindings::const_iterator it = localsByName_.find(identifier);
	if (it == localsByName_.end())
	{
		throw CompilerBug("Cannot get pointer for an unbound local identifier: '" + identifier.name() + "'");
	}
	return localsByName_[identifier];
}

Bindings::Mapping &Bindings::globals()
{
    return *globalsByName_;
}

Bindings::Mapping &Bindings::mappingFor(RefType refType)
{
    switch(refType)
    {
    case Local:
        return localsByName_;
    case Global:
        return *globalsByName_;
    case Closure:
        if (!closedValuesByName_)
        {
             throw CompilerBug("No closedValuesByName_");
        }
        return *closedValuesByName_;
    default:
        int rawValue = refType;
        throw CompilerBug("Unhandled refType " + str(rawValue));
    }
}

const Bindings::Mapping &Bindings::mappingFor(RefType refType) const
{
    switch(refType)
    {
    case Local:
        return localsByName_;
    case Global:
        return *globalsByName_;
    case Closure:
        if (!closedValuesByName_)
        {
             throw CompilerBug("No closedValuesByName_");
        }
        return *closedValuesByName_;
    default:
        int rawValue = refType;
        throw CompilerBug("Unhandled refType " + str(rawValue));
    }
}

Bindings::ValuePtr makeValue(const Value &value)
{
	return Bindings::ValuePtr(new Value(value));
}
    
std::ostream &operator<<(std::ostream &out, Bindings::RefType refType)
{
    switch(refType)
    {
    case Bindings::Local:
        return out << "refType(local)";
    case Bindings::Global:
        return out << "refType(global)";
    case Bindings::Closure:
        return out << "refType(closure)";
    default:
        int rawValue = refType;
        throw CompilerBug("Unhandled refType " + str(rawValue));
    }
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

Declarations::Declarations(const Bindings::Mapping &globalScope)
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

