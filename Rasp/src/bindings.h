#ifndef BINDINGS_H
#define BINDINGS_H

#include <map>

#include "value.h"
#include "identifier.h"

class Bindings
{
    typedef std::map<Identifier, Value> Mapping;
public:
    typedef Mapping::const_iterator const_iterator;
    
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator find(const Identifier &name) const;
    
    Value &operator[](const Identifier &name);
private:
    Mapping valuesByName;
};

class Scope
{
public:
	void add(const Identifier &identifer);
	bool isDefined(const Identifier &identifier) const;

private:
	std::vector<Identifier> declarations;
};

enum IdentifierDefinition {
	IDENTIFIER_DEFINITION_UNDEFINED,
	IDENTIFIER_DEFINITION_LOCAL,
	IDENTIFIER_DEFINITION_CLOSURE,
	IDENTIFIER_DEFINITION_GLOBAL,
};

class Declarations
{
public:
	Declarations();
	Declarations(const Bindings &globalScope);

	Declarations newScope();

	void add(const Identifier &identifer);
	bool isDefined(const Identifier &identifier) const;
	IdentifierDefinition checkIdentifier(const Identifier &identifier) const;

private:
	std::vector<Scope> innerToOuterScopes;
};

#endif
