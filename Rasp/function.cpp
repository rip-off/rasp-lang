#include "function.h"

#include "api.h"

Value Function::call(const Arguments &arguments) const
{
	return functionPointer(arguments);
}
