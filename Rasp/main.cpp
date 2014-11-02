#include <string>
#include <fstream>
#include <iostream>
#include <iterator>

#include "repl.h"
#include "lexer.h"
#include "parser.h"
#include "exceptions.h"
#include "unit_tests.h"
#include "interpreter.h"
#include "standard_library.h"

int main(int argc, const char **argv)
{
	Bindings bindings = bindStandardLibrary();
	Interpreter interpreter(bindings);

	runUnitTests(interpreter);

	if (argc < 2)
	{
		repl(interpreter);
	}
	else
	{
		for (int i = 1 ; i < argc ; ++i) 
		{
			std::fstream file(argv[i]);
			if (file)
			{
				std::string contents(
					// Extra parens for "most vexing parse"
					(std::istreambuf_iterator<char>(file)), 
					std::istreambuf_iterator<char>());
				try
				{
					Token token = lex(contents);
					InstructionList instructions = parse(token, bindings);
					Value result = interpreter.exec(instructions);
					std::cout << argv[i] << ": " << result << '\n';
				}
				catch(const LexError &e)
				{
					std::cerr << "Lex error: " << e.what() << '\n';
				}
				catch(const ParseError &e)
				{
					std::cerr << "Parse error: " << e.what() << '\n';
				}
				catch(const ExecutionError &e)
				{
					std::cerr << "Execution error: " << e.what() << '\n';
				}
				catch(const RaspError &e)
				{
					std::cerr << "General error: " << e.what() << '\n';
				}
				catch(const std::exception &error)
				{
					std::cerr << "Internal Error: " << error.what() << std::endl;
				}
			}
			else
			{
				std::cerr << "Failed to load " << argv[i] << '\n';
			}
		}
	}
}
