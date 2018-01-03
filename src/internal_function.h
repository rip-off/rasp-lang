#ifndef INTERNAL_FUNCTION
#define INTERNAL_FUNCTION

#include "function.h"
#include "instruction.h"

class InternalFunction : public Function
{
public:
	InternalFunction(
		const SourceLocation &sourceLocation,
		const Identifier &name,
		const std::vector<Identifier> &parameters, 
		const InstructionList &instructionList);

	virtual Function *clone() const;
	virtual Value call(CallContext &) const;
	virtual	const std::string &name() const;
	virtual	const SourceLocation &sourceLocation() const;

private:
	SourceLocation sourceLocation_;
	Identifier name_;
	std::vector<Identifier> parameters_;
	InstructionList instructionList_;
};

#endif

