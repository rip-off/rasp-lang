#include "api.h"

ExternalFunction::ExternalFunction(const std::string &name, const SourceLocation &sourceLocation, RawFunction *rawFunction)
:
	name_(name), 
	sourceLocation_(sourceLocation),
	rawFunction(rawFunction)
{
}

const std::string &ExternalFunction::name() const
{
	return name_;
}

const SourceLocation &ExternalFunction::sourceLocation() const
{
	return sourceLocation_;
}

Function *ExternalFunction::clone() const
{
	return new ExternalFunction(name_, sourceLocation_, rawFunction);
}

Value ExternalFunction::call(CallContext &context) const
{
	return rawFunction(context);
}

PureExternalFunction::PureExternalFunction(const std::string &name, const SourceLocation &sourceLocation, RawFunction *rawFunction)
:
	name_(name), 
	sourceLocation_(sourceLocation),
	rawFunction(rawFunction)
{
}

const std::string &PureExternalFunction::name() const
{
	return name_;
}

const SourceLocation &PureExternalFunction::sourceLocation() const
{
	return sourceLocation_;
}

Function *PureExternalFunction::clone() const
{
	return new PureExternalFunction(name_, sourceLocation_, rawFunction);
}

Value PureExternalFunction::call(CallContext &context) const
{
	return rawFunction(context.arguments());
}

ApiReg::ApiReg(const std::string &name, const SourceLocation &sourceLocation, ExternalFunction::RawFunction *rawFunction)
	: name_(name), function_(new ExternalFunction(name, sourceLocation, rawFunction))
{
}

ApiReg::ApiReg(const std::string &name, const SourceLocation &sourceLocation, PureExternalFunction::RawFunction *rawFunction)
	: name_(name), function_(new PureExternalFunction(name, sourceLocation, rawFunction))
{
}

const std::string &ApiReg::name() const 
{ 
	return name_; 
}

const Function &ApiReg::function() const
{ 
	return *function_; 
}

