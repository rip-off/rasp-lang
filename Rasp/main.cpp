#include <map>
#include <string>
#include <iostream>

#include "lexer.h"
#include "parser.h"
#include "exceptions.h"
#include "unit_tests.h"
#include "interpreter.h"
#include "standard_library.h"

int main()
{
	Bindings bindings = bindStandardLibrary();
	Interpreter interpreter(bindings);

	runUnitTests(interpreter);

	std::cout << "Enter some code:\n";
	std::string line;
	while((std::cout << " > ") && std::getline(std::cin, line) && !(line == "quit" || line == "exit"))
	{
		try
		{
			Token token = lex(line);
			InstructionList instructions = parse(token, bindings);
			std::cout << interpreter.exec(instructions) << std::endl;
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
}
