#ifndef BINDINGS_H
#define BINDINGS_H

#include <map>
#include <memory>

#include "value.h"
#include "identifier.h"

class Bindings
{
public:
    typedef std::shared_ptr<Value> ValuePtr;
    typedef std::map<Identifier, ValuePtr> Mapping;
    typedef Mapping::const_iterator const_iterator;
    enum RefType {
        Local,
        Global,
        Closure
    };

    Bindings(Mapping *globalsByName);

    Bindings(Mapping *globalsByName, Mapping *closedValuesByName);

    // Value should be bound
    const Value &get(RefType refType, const Identifier &identifier) const;

    // Value should be bound
    void set(RefType refType, const Identifier &identifier, const Value &value);

    // Value should not be bound
    void init(RefType refType, const Identifier &identifier, const Value &value);
    
    // Value should not be bound
    void initLocal(const Identifier &identifier, const Value &value);

    ValuePtr &getPointer(const Identifier &identifier);

private:
    // Deliberately private & unimplemented
    Bindings(const Bindings &);
    Bindings &operator=(const Bindings &);

    Mapping localsByName_;
    Mapping *globalsByName_;
    Mapping *closedValuesByName_;

    Mapping &mappingFor(RefType refType);
    const Mapping &mappingFor(RefType refType) const;
};

Bindings::ValuePtr makeValue(const Value &value);

std::ostream &operator<<(std::ostream &out, Bindings::RefType refType);

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
	Declarations(const Bindings::Mapping &globalScope);

	Declarations newScope();

	void add(const Identifier &identifer);
	bool isDefined(const Identifier &identifier) const;
	IdentifierDefinition checkIdentifier(const Identifier &identifier) const;

private:
	std::vector<Scope> innerToOuterScopes;
};

#endif
