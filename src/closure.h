#ifndef CLOSURE_H
#define CLOSURE_H

#include "function.h"
#include <memory>

class Closure : public Function
{
public:
	Closure(const Function &function, const Bindings::Mapping &closedValuesByName);
	virtual Closure *clone() const;
	virtual Value call(CallContext &) const;
	virtual	const std::string &name() const;
	virtual	const SourceLocation &sourceLocation() const;
private:
	std::unique_ptr<Function> innerFunction_;
	Bindings::Mapping closedValuesByName_;
};

#endif

