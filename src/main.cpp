#include <string>
#include <vector>
#include <iostream>

#include "repl.h"
#include "settings.h"
#include "compiler.h"
#include "interpreter.h"

#include "standard_math.h"
#include "standard_library.h"

#include "unit_tests.h"
#include "lexer_unit_tests.h"

typedef std::vector<std::string> ArgumentList;

void printUsage()
{
	std::cout << "Usage: <file names to run...>\n";
	std::cout << "\n";
	std::cout << "Options:\n";
	std::cout << " --repl: Run REPL (read, eval, print, loop)\n";
	std::cout << " --trace: Trace program execution\n";
	std::cout << " --unit-tests: Run unit test suite\n";
	std::cout << " --print-ast: Print Abstract Syntax Tree\n";
	std::cout << " --print-instructions: Print Generated Instructions\n";
	std::cout << " --help: Print this help message\n";
}

ArgumentList gatherArguments(int argc, const char **argv, Settings &settings)
{
	ArgumentList args;
	for (int i = 1 ; i < argc ; ++i)
	{
		std::string argument = argv[i];
		if (argument == "--help")
		{
			printUsage();
			std::exit(0);
		}
		else if (argument == "--repl")
		{
			settings.repl = true;
		}
		else if (argument == "--trace")
		{
			settings.trace = true;
		}
		else if (argument == "--unit-tests")
		{
			settings.unitTests = true;
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

int main(int argc, const char **argv)
{
	Settings settings;
	ArgumentList args = gatherArguments(argc, argv, settings);

	if (settings.unitTests)
	{
		return runLexerUnitTests() + runUnitTests(settings);
	}

	Interpreter::Globals globals;
	standardMath(globals);
	standardLibrary(globals);
	Interpreter interpreter(globals, settings);

	for (ArgumentList::const_iterator it = args.begin() ; it != args.end() ; ++it)
	{
		execute(interpreter, *it, settings);
	}

	if (settings.repl)
	{
		repl(interpreter, settings);
	}
	else if (args.empty())
	{
		printUsage();
	}
}

