#include "repl.h"

#include <string>
#include <iostream>

#include "lexer.h"
#include "parser.h"
#include "exceptions.h"
#include "interpreter.h"

void repl(Interpreter &interpreter, const Settings &settings)
{
	std::cout << "Enter some code, or type \'exit\' when finished:\n";
	std::string line;
	while((std::cout << " > ") && std::getline(std::cin, line) && !(line == "quit" || line == "exit"))
	{
		try
		{
			Token token = lex(line);
			std::vector<Identifier> declarations = interpreter.declarations();
			InstructionList instructions = parse(token, declarations, settings);
			Value result = interpreter.exec(instructions);
			std::cout << " < " << result << std::endl;
		}
		catch(const LexError &e)
		{
			std::cerr << "Lex error at line " << e.line() << ": " << e.what() << '\n';
		}
		catch(const ParseError &e)
		{
			std::cerr << "Parse error at line " << e.line() << ": " << e.what() << '\n';
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
