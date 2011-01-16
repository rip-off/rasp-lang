#include <map>
#include <string>
#include <iostream>

#include "parser.h"
#include "compiler.h"
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
			Token token = parse(line);
			InstructionList instructions = compile(token, bindings);
			std::cout << interpreter.exec(instructions) << std::endl;
		}
		catch(const ParseError &e)
		{
			std::cerr << "Parser error: " << e.what() << '\n';
		}
		catch(const CompileError &e)
		{
			std::cerr << "Compiler error: " << e.what() << '\n';
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
