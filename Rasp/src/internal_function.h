#ifndef INTERNAL_FUNCTION
#define INTERNAL_FUNCTION

#include "function.h"
#include "instruction.h"

class InternalFunction : public Function
{
public:
	InternalFunction(
		const std::string &name, 
		const std::vector<Identifier> &parameters, 
		const InstructionList &instructionList);

	virtual Function *clone() const;
	virtual Value call(CallContext &) const;
	virtual	const std::string &name() const;

private:
	std::string name_;
	std::vector<Identifier> parameters_;
	InstructionList instructionList_;
};

#endif
