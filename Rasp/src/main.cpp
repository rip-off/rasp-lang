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
		if (argument == "--help")
		{
			std::cout << "Usage: <file names to run...>\n";
			std::cout << "\n";
			std::cout << "When no file names are passed, a REPL is started\n";
			std::cout << "\n";
			std::cout << "Options:\n";
			std::cout << " --trace: Trace program execution\n";
			std::cout << " --print-ast: Print Abstract Syntax Tree\n";
			std::cout << " --print-instructions: Print Generated Instructions\n";
			std::cout << " --help: Print this help message\n";
			std::exit(0);
		}
		else if (argument == "--trace")
		{
			settings.trace = true;
		}
		else if (argument == "--print-ast")
		{
			settings.printSyntaxTree = true;
		}
		else if (argument == "--print-instructions")
		{
			settings.printInstructions = true;
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

