#include "parser.h"

#include <iostream>
#include <algorithm>

#include "token.h"
#include "bindings.h"
#include "settings.h"
#include "exceptions.h"
#include "internal_function.h"

namespace
{
	bool isDefined(const std::vector<Identifier> &declarations, const Identifier &identifier)
	{
		return std::find(declarations.begin(), declarations.end(), identifier) != declarations.end();
	}

	Identifier tryMakeIdentifier(unsigned line, const std::string &name)
	{
		if (!Identifier::isValid(name))
		{
			throw ParseError(line, "Illegal identifier '" + name + "'");
		}
		return Identifier(name);
	}

	void parse(const Token &token, std::vector<Identifier> &declarations, InstructionList &instructions);

	void handleList(const Token &token, std::vector<Identifier> &declarations, InstructionList &instructions)
	{
		const Token::Children &children = token.children();
		if(children.empty())
		{
			throw ParseError(token.line(), "Empty list is not allowed");
		}

		const Token &firstChild = children.front();		
		if(firstChild.type() == Token::Keyword)
		{
			const std::string &keyword = firstChild.string();
			if(keyword == "while")
			{
				if(children.size() == 1)
				{
					throw ParseError(token.line(), "'while' expression is missing condition");
				}
				else if(children.size() == 2)
				{
					throw ParseError(token.line(), "'while' expression is missing code to execute");
				}
				unsigned previousInstructionCount = instructions.size();
				// Evaluate the conditional expression first
				parse(children[1], declarations, instructions);
				unsigned conditionExpressionInstructions = instructions.size() - previousInstructionCount;
				// Generate the list of instructions to be executed if branch is taken
				InstructionList tempInstructions;
				for(unsigned i = 2 ; i < children.size() ; ++i)
				{
					parse(children[i], declarations, tempInstructions);
				}
				unsigned bodyInstructions = tempInstructions.size();
				// Actual branch instruction
				// +1 for the loop instruction itself!
				instructions.push_back(Instruction::jump(bodyInstructions + 1));
				// Insert the remaining instructions into the stream
				instructions.insert(instructions.end(), tempInstructions.begin(), tempInstructions.end());
				// Return to loop start
				// +1 for jump instruction
				// +1 for this loop instruction itself!
				instructions.push_back(Instruction::loop(bodyInstructions + 1 + conditionExpressionInstructions + 1));
			}
			else if(keyword == "if")
			{
				if(children.size() == 1)
				{
					throw ParseError(token.line(), "Keyword 'if' expression is missing condition");
				}
				else if(children.size() == 2)
				{
					throw ParseError(token.line(), "Keyword 'if' expression is missing code to execute");
				}
				// Evaluate the conditional expression first
				parse(children[1], declarations, instructions);
				// Generate the list of instructions to be executed if branch is taken
				InstructionList tempInstructions;
				for(unsigned i = 2 ; i < children.size() ; ++i)
				{
					parse(children[i], declarations, tempInstructions);
				}
				// Actual branch instruction
				instructions.push_back(Instruction::jump(tempInstructions.size()));
				// Insert the remaining instructions into the stream
				instructions.insert(instructions.end(), tempInstructions.begin(), tempInstructions.end());
			}
			else if(keyword == "var")
			{
				if(children.size() == 2)
				{
					throw ParseError(token.line(), "Keyword 'var' declaration requires a name");
				}
				else if(children.size() != 3)
				{
					throw ParseError(token.line(), "Keyword 'var' missing initialisation value");
				}

				Identifier identifier = tryMakeIdentifier(children[1].line(), children[1].string());
				if (isDefined(declarations, identifier))
				{
					throw ParseError(token.line(), "Keyword 'var' identity '" + identifier.name() + "' already defined");
				}
				declarations.push_back(identifier);				
				parse(children[2], declarations, instructions);
				instructions.push_back(Instruction::assign(identifier.name()));
			}
			else if(keyword == "set")
			{
				if(children.size() == 2)
				{
					throw ParseError(token.line(), "Keyword 'set' variable assignment requires a name");
				}
				else if(children.size() != 3)
				{
					throw ParseError(token.line(), "Keyword 'set' missing assignment value");
				}
				Identifier identifier = tryMakeIdentifier(children[1].line(), children[1].string());
				if (!isDefined(declarations, identifier))
				{
					throw ParseError(token.line(), "Keyword 'set' identifier '" + identifier.name() + "' not defined");
				}	
				parse(children[2], declarations, instructions);
				instructions.push_back(Instruction::assign(identifier.name()));
			}
			else if(keyword == "defun")
			{
				if(children.size() == 2)
				{
					throw ParseError(token.line(), "Keyword 'defun' function requires a name");
				}
				else if(children.size() == 3)
				{
					throw ParseError(token.line(), "Keyword 'defun' function requires parameter lists");
				}
				else if(children.size() < 4)
				{
					throw ParseError(token.line(), "Keyword 'defun' function lacks a body");
				}

				Identifier identifier = tryMakeIdentifier(children[1].line(), children[1].string());
				if (isDefined(declarations, identifier))
				{
					throw ParseError(token.line(), "Keyword 'defun' identifier " + identifier.name() + " already defined");
				}
				declarations.push_back(identifier);
				
				if (children[2].type() != Token::List)
				{
					throw ParseError(token.line(), "Keyword 'defun' function parameter list is incorrect");
				}

				std::vector<Identifier> parameters;
				std::vector<Identifier> localDeclarations = declarations;
				
				const Token::Children &rawParameters = children[2].children();
				for (unsigned i = 0 ; i < rawParameters.size()  ; ++i)
				{
					if (rawParameters[i].type() != Token::Identifier)
					{
						throw ParseError(token.line(), "Keyword 'defun' function parameter " + str(i) + " is incorrect");
					}
					Identifier parameter = tryMakeIdentifier(rawParameters[i].line(), rawParameters[i].string());
					parameters.push_back(parameter);
					localDeclarations.push_back(parameter);
				}

				InstructionList tempInstructions;
				for(unsigned i = 3 ; i < children.size() ; ++i)
				{
					parse(children[i], localDeclarations, tempInstructions);
				}
				InternalFunction function(token.line(), identifier.name(), parameters, tempInstructions);
				instructions.push_back(Instruction::push(function));
				instructions.push_back(Instruction::assign(identifier.name()));
			}
			else
			{
				throw ParseError(token.line(), "Compiler bug: unhandled keyword '" + token.string() + "'");
			}
		}
		else
		{
			for(Token::Children::const_reverse_iterator i = children.rbegin() ; i != children.rend() ; ++i)
			{
				parse(*i, declarations, instructions);
			}
			// Call expects the number of arguments, so we must omit 1 element
			// This is because the function is the mandatory first element
			// TODO: check the first element is a function at compile time
			instructions.push_back(Instruction::call(children.size() - 1));
		}
	}

	void parse(const Token &token, std::vector<Identifier> &declarations, InstructionList &instructions)
	{
		const Token::Children &children = token.children();
		switch(token.type())
		{
		case Token::Nil:
			assert(children.empty());
			instructions.push_back(Instruction::push(Value::nil()));
			break;
		case Token::Root:
			assert(children.empty());
			// do nothing;
			break;
		case Token::List:
			{
				handleList(token, declarations, instructions);
			}
			break;
		case Token::String:
			assert(children.empty());
			instructions.push_back(Instruction::push(token.string()));
			break;
		case Token::Number:
			assert(children.empty());
			instructions.push_back(Instruction::push(to<int>(token.string())));
			break;
		case Token::Boolean:
			assert(children.empty());
			if (token.string() == "true")
			{
				instructions.push_back(Instruction::push(Value::boolean(true)));
			}
			else if (token.string() == "false")
			{
				instructions.push_back(Instruction::push(Value::boolean(false)));
			}
			else
			{
				// TODO: compiler bug class?
				throw ParseError(token.line(), "BUG: illegal boolean literal '" + token.string() + "'");
			}
			break;
		case Token::Keyword:
			{
				throw ParseError(token.line(), "Keyword '" + token.string() + "' must be first element of a list");
			}
			break;
		case Token::Identifier:
			{
				assert(children.empty());
				Identifier identifier = tryMakeIdentifier(token.line(), token.string());
				if (!isDefined(declarations, identifier))
				{
					throw ParseError(token.line(), "Variable '" + identifier.name() + "' not defined");
				}
				instructions.push_back(Instruction::ref(identifier));
			}
			break;
		}
	}

	void printTabs(int num)
	{
		while(num --> 0)
		{
			std::cout << '\t';
		}
	}

	void printTree(const Token &token, int level)
	{
		printTabs(level);
		const Token::Children &children = token.children();
		switch(token.type())
		{
		case Token::Nil:
			assert(children.empty());
			std::cout << "Nil\n";
			break;
		case Token::Root:
			std::cout << "Root {\n";
			for(Token::Children::const_iterator i = children.begin() ; i != children.end() ; ++i)
			{
				printTree(*i, level + 1);
			}
			printTabs(level);
			std::cout << "}";
			break;
		case Token::List:
			std::cout << "List {\n";
			for(Token::Children::const_iterator i = children.begin() ; i != children.end() ; ++i)
			{
				printTree(*i, level + 1);
			}
			printTabs(level);
			std::cout << "}";
			break;
		case Token::String:
			assert(children.empty());
			std::cout << "String(" << token.string() << ')' << '\n';
			break;
		case Token::Number:
			assert(children.empty());
			std::cout << "Number(" << token.string() << ')' << '\n';
			break;
		case Token::Boolean:
			assert(children.empty());
			std::cout << "Boolean(" << token.string() << ')' << '\n';
			break;
		case Token::Keyword:
			assert(children.empty());
			std::cout << token.string();
			break;
		case Token::Identifier:
			assert(children.empty());
			std::cout << "Identifier(" << token.string() << ')' << '\n';
			break;
		}
		std::cout << '\n';
	}
}

InstructionList parse(const Token &tree, std::vector<Identifier> &declarations, const Settings &settings)
{
	if (settings.verbose)
	{
		printTree(tree,1);
	}

	InstructionList result;
	assert(tree.type() == Token::Root);
	const Token::Children &children = tree.children();
	for(Token::Children::const_iterator it = children.begin() ; it != children.end() ; ++it)
	{
		parse(*it, declarations, result);
	}

	if (settings.verbose)
	{
		unsigned n = 0;
		std::cout << "Generated " << result.size() << " instructions:\n";
		for(InstructionList::const_iterator it = result.begin() ; it != result.end() ; ++it)
		{
			std::cout << (n + 1) << ": " << *it << '\n';
			n += 1;
		}
	}

	return result;
}
