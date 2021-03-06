#include "compiler.h"

#include <fstream>
#include <iostream>
#include <iterator>

#include "lexer.h"
#include "parser.h"
#include "settings.h"
#include "exceptions.h"
#include "interpreter.h"
#include "execution_error.h"

void execute(Interpreter &interpreter, const std::string &filename, const Settings &settings)
{
	std::fstream file(filename.c_str());
	if (file)
	{
		std::string contents(
			// Extra parens for "most vexing parse"
			(std::istreambuf_iterator<char>(file)), 
			std::istreambuf_iterator<char>());
		try
		{
			Token token = lex(filename, contents);
			Declarations declarations = interpreter.declarations();
			InstructionList instructions = parse(token, declarations, settings);
			interpreter.exec(instructions);
		}
		catch(const LexError &e)
		{
			std::cerr << "Lex error at " << e.sourceLocation() << " " << e.what() << '\n';
			printStackTrace(std::cerr, e);
		}
		catch(const ParseError &e)
		{
			std::cerr << "Parse error at " << e.sourceLocation() << " " << e.what() << '\n';
			printStackTrace(std::cerr, e);
		}
		catch(const ExecutionError &e)
		{
			std::cerr << "Execution error @ " << e.sourceLocation() << " " << e.what() << '\n';
			printStackTrace(std::cerr, e);
		}
		catch(const RaspError &e)
		{
			std::cerr << "General error in " << filename << ": " << e.what() << '\n';
			printStackTrace(std::cerr, e);
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
