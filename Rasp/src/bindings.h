#ifndef BINDINGS_H
#define BINDINGS_H

#include <map>

#include "value.h"
#include "identifier.h"

typedef std::map<Identifier, Value> Bindings;

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
