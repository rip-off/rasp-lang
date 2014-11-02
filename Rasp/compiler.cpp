#include "compiler.h"

#include <fstream>
#include <iostream>
#include <iterator>

#include "lexer.h"
#include "parser.h"
#include "settings.h"
#include "exceptions.h"
#include "interpreter.h"

void execute(Interpreter &interpreter, const std::string &filename, const Settings &settings)
{
	Bindings &bindings = interpreter.bindings();

	std::fstream file(filename.c_str());
	if (file)
	{
		std::string contents(
			// Extra parens for "most vexing parse"
			(std::istreambuf_iterator<char>(file)), 
			std::istreambuf_iterator<char>());
		try
		{
			Token token = lex(contents);
			InstructionList instructions = parse(token, bindings, settings);
			Value result = interpreter.exec(instructions);
			std::cout << filename << ": " << result << '\n';
		}
		catch(const LexError &e)
		{
			std::cerr << "Lex error in " << filename << " at line " << e.line() << ": " << e.what() << '\n';
		}
		catch(const ParseError &e)
		{
			std::cerr << "Parse error in " << filename << ": " << e.what() << '\n';
		}
		catch(const ExecutionError &e)
		{
			std::cerr << "Execution error in " << filename << ": " << e.what() << '\n';
		}
		catch(const RaspError &e)
		{
			std::cerr << "General error in " << filename << ": " << e.what() << '\n';
		}
		catch(const std::exception &error)
		{
			std::cerr << "Internal Error in " << filename << ": " << error.what() << std::endl;
		}
	}
	else
	{
		std::cerr << "Failed to load " << filename << '\n';
	}
}
