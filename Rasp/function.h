#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include "common.h"

class Function
{
public:
	virtual ~Function();
	virtual Function *clone() const = 0;
	virtual Value call(CallContext &) const = 0;
	virtual	const std::string &name() const = 0;

protected:
	Function();

private:
	// Deliberately private and unimplemented
	Function(const Function &);
	Function &operator=(const Function &);
};


#endif
