#include "api.h"

ExternalFunction::ExternalFunction(const std::string &name, RawFunction *rawFunction)
:
	name_(name), 
	rawFunction(rawFunction)
{
}

const std::string &ExternalFunction::name() const
{
	return name_;
}

Function *ExternalFunction::clone() const
{
	return new ExternalFunction(name_, rawFunction);
}

Value ExternalFunction::call(CallContext &context) const
{
	return rawFunction(context);
}

PureExternalFunction::PureExternalFunction(const std::string &name, RawFunction *rawFunction)
:
	name_(name), 
	rawFunction(rawFunction)
{
}

const std::string &PureExternalFunction::name() const
{
	return name_;
}

Function *PureExternalFunction::clone() const
{
	return new PureExternalFunction(name_, rawFunction);
}

Value PureExternalFunction::call(CallContext &context) const
{
	return rawFunction(context.arguments());
}

ApiReg::ApiReg(const std::string &name, ExternalFunction::RawFunction *rawFunction) 
	: name_(name), function_(new ExternalFunction(name, rawFunction))
{
}

ApiReg::ApiReg(const std::string &name, PureExternalFunction::RawFunction *rawFunction) 
	: name_(name), function_(new PureExternalFunction(name, rawFunction))
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

