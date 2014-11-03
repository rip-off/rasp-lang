#include <string>
#include <vector>
#include <iostream>

#include "repl.h"
#include "settings.h"
#include "compiler.h"
#include "unit_tests.h"
#include "interpreter.h"
#include "standard_library.h"

typedef std::vector<std::string> ArgumentList;

ArgumentList gatherArguments(int argc, const char **argv, Settings &settings)
{
	ArgumentList args;
	for (int i = 1 ; i < argc ; ++i)
	{
		std::string argument = argv[i];
		if (argument == "--verbose")
		{
			std::cout << "INFO: Verbose mode enabled!\n";
			settings.verbose = true;
		}
		else
		{
			args.push_back(argument);
		}
	}
	return args;
}

void unitTests()
{
	Settings settings;
	Bindings bindings = bindStandardLibrary();
	Interpreter interpreter(bindings, settings);
	runUnitTests(interpreter);
}

int main(int argc, const char **argv)
{
	unitTests();

	Settings settings;
	ArgumentList args = gatherArguments(argc, argv, settings);	

	// TODO: Probably don't want to share them between files
	Bindings bindings = bindStandardLibrary();
	Interpreter interpreter(bindings, settings);

	if (args.empty())
	{
		repl(interpreter, settings);
	}
	else
	{
		for (ArgumentList::const_iterator it = args.begin() ; it != args.end() ; ++it) 
		{
			execute(interpreter, *it, settings);
		}
	}
}

