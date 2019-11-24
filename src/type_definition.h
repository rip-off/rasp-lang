#ifndef TYPE_DEFINITION_H
#define TYPE_DEFINITION_H

#include <string>
#include <vector>
#include <memory>

#include "identifier.h"

class TypeDefinition
{
private:
	Identifier name_;
	std::vector<Identifier> memberNames_;

public:
	TypeDefinition(const Identifier &name, const std::vector<Identifier> &memberNames)
	:
		name_(name),
		memberNames_(memberNames)
	{
	}

	const std::string &name() const
	{
		return name_.name();
	}

	const std::vector<Identifier> &memberNames() const
	{
		return memberNames_;
	}

private:
	// noncopyable: unimplemented
	TypeDefinition(const TypeDefinition &);
	TypeDefinition &operator=(const TypeDefinition &);
};

typedef std::shared_ptr<TypeDefinition> TypePointer;

#endif

