#ifndef API_H
#define API_H

#include <vector>
#include <memory>
#include <algorithm>

#include "utils.h"
#include "value.h"
#include "common.h"
#include "bindings.h"
#include "function.h"

class ExternalFunction : public Function
{
public:
	typedef Value RawFunction(CallContext &);

	ExternalFunction(const std::string &name, const SourceLocation &sourceLocation, RawFunction *rawFunction);

	virtual Function *clone() const;
	virtual Value call(CallContext &) const;
	virtual	const std::string &name() const;
	virtual	const SourceLocation &sourceLocation() const;

private:
	std::string name_;
	SourceLocation sourceLocation_;
	RawFunction *rawFunction;
};

// Pure from the point of the view of the interpreter
class PureExternalFunction : public Function
{
public:
	typedef Value RawFunction(const Arguments &args);

	PureExternalFunction(const std::string &name, const SourceLocation &sourceLocation, RawFunction *rawFunction);

	virtual Function *clone() const;
	virtual Value call(CallContext &) const;
	virtual	const std::string &name() const;
	virtual	const SourceLocation &sourceLocation() const;

private:
	std::string name_;
	SourceLocation sourceLocation_;
	RawFunction *rawFunction;
};

struct ApiReg
{
public:
	ApiReg(const std::string &name, const SourceLocation &sourceLocation, ExternalFunction::RawFunction *rawFunction) ;
	ApiReg(const std::string &name, const SourceLocation &sourceLocation, PureExternalFunction::RawFunction *rawFunction);

	const std::string &name() const;
	const Function &function() const;

private:
	std::string name_;
	std::unique_ptr<Function> function_;
};

template<int N>
void registerBindings(Bindings &bindings, const ApiReg (&registry)[N])
{
	for(const ApiReg *current = registry ; current != registry + N ; ++current)
	{
		Identifier identifier = Identifier(current->name());
		Value functionValue = Value::function(current->function());
		bindings.insert(std::make_pair(identifier, functionValue));
	}
}

#endif
