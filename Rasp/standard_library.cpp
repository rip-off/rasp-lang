#include "standard_library.h"

#include <ctime>
#include <iostream>

// Platform specific includes for sleep()
// TODO: move platform specific implementations to separate compilation unit?
#if defined(WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
// TODO: to use this we must enable language extensions in MSVC, see if we can work around this...
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#include "api.h"
#include "exceptions.h"

namespace
{

	Value plus(const Arguments &arguments)
	{
		int result = 0;
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			if(!i->isNumber())
			{
				throw ExecutionError("Expected numeric argument");
			}
			result += i->number();
		}
		return result;
	}

	Value mul(const Arguments &arguments)
	{
		int result = 1;
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			if(!i->isNumber())
			{
				throw ExecutionError("Expected numeric argument");
			}
			result *= i->number();
		}
		return result;
	}

	Value sub(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExecutionError("Expected 2 numeric arguments");
		}
		return arguments[0].number() - arguments[1].number();
	}

	Value div(const Arguments &arguments)
	{
		if(arguments.size() != 2 || !(arguments[0].isNumber() && arguments[1].isNumber()))
		{
			throw ExecutionError("Expected 2 numeric arguments");
		}
		return arguments[0].number() / arguments[1].number();
	}

	Value print(const Arguments &arguments)
	{
		for(Arguments::const_iterator i = arguments.begin() ; i != arguments.end() ; ++i)
		{
			// TODO: remove automatic space when we support character/string literals.
			std::cout << *i << ' ';
		}
		std::cout << '\n';
		return Value();
	}

	Value sleep(const Arguments &arguments)
	{
		if(arguments.size() != 1 || !arguments[0].isNumber())
		{
			throw ExecutionError("Expected 1 numeric argument");
		}
	
		int milliseconds = arguments[0].number();
#if defined(WIN32)
		Sleep(milliseconds);
#elif defined(__linux__)
		usleep(milliseconds * 1000);
#else
#error "please implement sleep() on your platform"
#endif
		return Value();
	}

	Value time(const Arguments &arguments)
	{
		if(!arguments.empty())
		{
			throw ExecutionError("Expect no arguments");
		}
		return static_cast<int>(std::time(0));
	}

	Value println(const Arguments &arguments)
	{
		print(arguments);
		std::cout << '\n';
		return Value();
	}

#define ENTRY(X) ApiReg(#X, &X)

	const ApiReg registry[] = 
	{
		ApiReg("+", &plus),
		ApiReg("-", &sub),
		ApiReg("/", &div),
		ApiReg("*", &mul),
		ENTRY(time),
		ENTRY(sleep),
		ENTRY(print),
		ENTRY(println),
	};

#undef ENTRY
}



Bindings bindStandardLibrary()
{
	Bindings result;
	registerBindings(result, registry);
	return result;
}
