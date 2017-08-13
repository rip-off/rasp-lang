#include "parser.h"

#include <iostream>
#include <algorithm>

#include "bug.h"
#include "token.h"
#include "escape.h"
#include "bindings.h"
#include "settings.h"
#include "exceptions.h"
#include "type_definition.h"
#include "internal_function.h"

namespace
{
	void printInstructions(const InstructionList &instructions)
	{
		std::cout << "Generated " << instructions.size() << " instructions:\n";
		unsigned n = 0;
		for(InstructionList::const_iterator it = instructions.begin() ; it != instructions.end() ; ++it)
		{
			std::cout << (n + 1) << ": " << *it << '\n';
			n += 1;
		}
		std::cout << '\n';
	}

	Identifier tryMakeIdentifier(const Token &token)
	{
		// TODO: differentiate between identifiers and member access?
		//assert(token.children().empty());
		const std::string &name = token.string();
		if (!Identifier::isValid(name))
		{
			throw ParseError(token.sourceLocation(), "Illegal identifier '" + name + "'");
		}
		return Identifier(name);
	}

	std::vector<Identifier> getClosedValues(const InstructionList &instructions)
	{
		std::vector<Identifier> result;
		for (const Instruction &instruction : instructions)
		{
			if (instruction.type() == Instruction::RefClosure)
			{
				Identifier identifier = Identifier(instruction.value().string());
				if (std::find(result.begin(), result.end(), identifier) == result.end())
				{
					result.push_back(identifier);
				}
			}
		}
		return result;
	}

	void parse(const Token &token, Declarations &declarations, InstructionList &instructions, const Settings &settings);

	void handleList(const Token &token, Declarations &declarations, InstructionList &instructions, const Settings &settings)
	{
		const Token::Children &children = token.children();
		if(children.empty())
		{
			throw ParseError(token.sourceLocation(), "Empty list is not allowed");
		}

		const Token &firstChild = children.front();		
		if(firstChild.type() == Token::Keyword)
		{
			const std::string &keyword = firstChild.string();
			if(keyword == "while")
			{
				if(children.size() == 1)
				{
					throw ParseError(token.sourceLocation(), "'while' expression is missing condition");
				}
				else if(children.size() == 2)
				{
					throw ParseError(token.sourceLocation(), "'while' expression is missing code to execute");
				}
				unsigned previousInstructionCount = instructions.size();
				// Evaluate the conditional expression first
				parse(children[1], declarations, instructions, settings);
				unsigned conditionExpressionInstructions = instructions.size() - previousInstructionCount;
				// Generate the list of instructions to be executed if branch is taken
				InstructionList tempInstructions;
				for(unsigned i = 2 ; i < children.size() ; ++i)
				{
					parse(children[i], declarations, tempInstructions, settings);
				}
				unsigned bodyInstructions = tempInstructions.size();
				// Actual branch instruction
				// +1 for the loop instruction itself!
				instructions.push_back(Instruction::jump(token.sourceLocation(), bodyInstructions + 1));
				// Insert the remaining instructions into the stream
				instructions.insert(instructions.end(), tempInstructions.begin(), tempInstructions.end());
				// Return to loop start
				// +1 for jump instruction
				// +1 for this loop instruction itself!
				instructions.push_back(Instruction::loop(token.sourceLocation(), bodyInstructions + 1 + conditionExpressionInstructions + 1));
			}
			else if(keyword == "if")
			{
				if(children.size() == 1)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'if' expression is missing condition");
				}
				else if(children.size() == 2)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'if' expression is missing code to execute");
				}
				// Evaluate the conditional expression first
				parse(children[1], declarations, instructions, settings);
				// Generate the list of instructions to be executed if branch is taken
				InstructionList tempInstructions;
				for(unsigned i = 2 ; i < children.size() ; ++i)
				{
					parse(children[i], declarations, tempInstructions, settings);
				}
				// Actual branch instruction
				instructions.push_back(Instruction::jump(token.sourceLocation(), tempInstructions.size()));
				// Insert the remaining instructions into the stream
				instructions.insert(instructions.end(), tempInstructions.begin(), tempInstructions.end());
			}
			else if(keyword == "var")
			{
				if(children.size() == 1)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'var' declaration requires a name");
				}
				else if(children.size() == 2)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'var' missing initialisation value");
				}
				else if(children.size() > 3)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'var' too many arguments");
				}

				Identifier identifier = tryMakeIdentifier(children[1]);
				if (declarations.isDefined(identifier))
				{
					throw ParseError(token.sourceLocation(), "Keyword 'var' identifier '" + identifier.name() + "' already defined");
				}
				parse(children[2], declarations, instructions, settings);
				declarations.add(identifier);
				// TODO: "init" variation?
				switch(declarations.checkIdentifier(identifier))
				{
				case IDENTIFIER_DEFINITION_UNDEFINED:
					throw ParseError(token.sourceLocation(), "Identifier '" + identifier.name() + "' not defined");
					break;
				case IDENTIFIER_DEFINITION_LOCAL:
					instructions.push_back(Instruction::assignLocal(token.sourceLocation(), identifier.name()));
					break;
				case IDENTIFIER_DEFINITION_CLOSURE:
					throw CompilerBug("Identifier '" + identifier.name() + "' unexpectedly classified as closure");
					break;
				case IDENTIFIER_DEFINITION_GLOBAL:
					instructions.push_back(Instruction::assignGlobal(token.sourceLocation(), identifier.name()));
					break;
				default:
					throw CompilerBug("Failed to classify identifier " + identifier.name() + " at " + str(token.sourceLocation()));
				}
			}
			else if(keyword == "set")
			{
				if(children.size() == 2)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'set' variable assignment requires a name");
				}
				else if(children.size() != 3)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'set' missing assignment value");
				}
				Identifier identifier = tryMakeIdentifier(children[1]);
				parse(children[2], declarations, instructions, settings);
				switch(declarations.checkIdentifier(identifier))
				{
				case IDENTIFIER_DEFINITION_UNDEFINED:
					throw ParseError(token.sourceLocation(), "Identifier '" + identifier.name() + "' not defined");
					break;
				case IDENTIFIER_DEFINITION_LOCAL:
					instructions.push_back(Instruction::assignLocal(token.sourceLocation(), identifier.name()));
					break;
				case IDENTIFIER_DEFINITION_CLOSURE:
					instructions.push_back(Instruction::assignClosure(token.sourceLocation(), identifier.name()));
					break;
				case IDENTIFIER_DEFINITION_GLOBAL:
					instructions.push_back(Instruction::assignGlobal(token.sourceLocation(), identifier.name()));
					break;
				default:
					throw CompilerBug("Failed to classify identifier " + identifier.name() + " at " + str(token.sourceLocation()));
				}
			}
			else if(keyword == "type")
			{
				if(children.empty())
				{
					throw ParseError(token.sourceLocation(), "Keyword 'type' requires a name");
				}
				else if(children.size() == 1)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'type' function requires members");
				}
				Identifier identifier = tryMakeIdentifier(children[1]);
				if (declarations.isDefined(identifier))
				{
					throw ParseError(token.sourceLocation(), "Keyword 'type' identifier " + identifier.name() + " already defined");
				}

				std::vector<Identifier> memberNames;
				for (int i = 2 ; i < children.size() ; ++i)
				{
					Identifier identifier = tryMakeIdentifier(children[i]);
					memberNames.push_back(identifier);
				}

				TypeDefinition typeDefinition = { identifier.name(), memberNames };
				instructions.push_back(Instruction::push(token.sourceLocation(), Value::typeDefinition(typeDefinition)));
				// TODO: local? global?
				instructions.push_back(Instruction::assignGlobal(token.sourceLocation(), identifier.name()));

				// For the moment, do not allow recursive types
				declarations.add(identifier);
			}
			else if(keyword == "defun")
			{
				// TODO: error messages are incorrect
				if(children.size() == 2)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'defun' function requires a name");
				}
				else if(children.size() == 3)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'defun' function requires parameter lists");
				}
				else if(children.size() < 4)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'defun' function lacks a body");
				}

				Identifier identifier = tryMakeIdentifier(children[1]);
				if (declarations.isDefined(identifier))
				{
					throw ParseError(token.sourceLocation(), "Keyword 'defun' identifier " + identifier.name() + " already defined");
				}
				// Allow recursion
				declarations.add(identifier);
				
				if (children[2].type() != Token::List)
				{
					throw ParseError(token.sourceLocation(), "Keyword 'defun' function parameter list is incorrect");
				}

				std::vector<Identifier> parameters;
				Declarations localDeclarations = declarations.newScope();
				
				const Token::Children &rawParameters = children[2].children();
				for (unsigned i = 0 ; i < rawParameters.size() ; ++i)
				{
					if (rawParameters[i].type() != Token::Identifier)
					{
						throw ParseError(token.sourceLocation(), "Keyword 'defun' function parameter " + str(i) + " is incorrect");
					}
					Identifier parameter = tryMakeIdentifier(rawParameters[i]);
					parameters.push_back(parameter);
					localDeclarations.add(parameter);
				}

				InstructionList tempInstructions;
				for(unsigned i = 3 ; i < children.size() ; ++i)
				{
					parse(children[i], localDeclarations, tempInstructions, settings);
				}

				std::vector<Identifier> closedValues = getClosedValues(tempInstructions);
				if (closedValues.empty())
				{
					InternalFunction function(token.sourceLocation(), identifier.name(), parameters, tempInstructions);
					instructions.push_back(Instruction::push(token.sourceLocation(), Value::function(function)));
				}
				else
				{
					for (unsigned i = 0 ; i < closedValues.size() ; ++i)
					{
						parameters.insert(parameters.begin(), closedValues[i]);
						instructions.push_back(Instruction::refClosure(token.sourceLocation(), closedValues[i]));
					}
					InternalFunction function(token.sourceLocation(), identifier.name(), parameters, tempInstructions);
					instructions.push_back(Instruction::push(token.sourceLocation(), Value::function(function)));
					instructions.push_back(Instruction::capture(token.sourceLocation(), closedValues.size()));
				}

				switch(declarations.checkIdentifier(identifier))
				{
				case IDENTIFIER_DEFINITION_UNDEFINED:
					throw ParseError(token.sourceLocation(), "Identifier '" + identifier.name() + "' not defined");
					break;
				case IDENTIFIER_DEFINITION_LOCAL:
					instructions.push_back(Instruction::assignLocal(token.sourceLocation(), identifier.name()));
					break;
				case IDENTIFIER_DEFINITION_CLOSURE:
					throw CompilerBug("Identifier '" + identifier.name() + "' unexpectedly classified as closure");
					break;
				case IDENTIFIER_DEFINITION_GLOBAL:
					instructions.push_back(Instruction::assignGlobal(token.sourceLocation(), identifier.name()));
					break;
				default:
					throw CompilerBug("Failed to classify identifier " + identifier.name() + " at " + str(token.sourceLocation()));
				}

				if (settings.printInstructions)
				{
					std::cout << "Function (" << identifier.name();
					for (unsigned i = 0 ; i < parameters.size()  ; ++i)
					{
						std::cout << " " << parameters[i].name();
					}
					std::cout << ") @ " << token.sourceLocation() << '\n';
					printInstructions(tempInstructions);
				}
			}
			else
			{
				throw CompilerBug("unhandled keyword '" + token.string() + "' at line " + str(token.sourceLocation()));
			}
		}
		else
		{
			for(Token::Children::const_reverse_iterator i = children.rbegin() ; i != children.rend() ; ++i)
			{
				parse(*i, declarations, instructions, settings);
			}
			// Call expects the number of arguments, so we must omit 1 element
			// This is because the function is the mandatory first element
			instructions.push_back(Instruction::call(token.sourceLocation(), children.size() - 1));
		}
	}

	void parse(const Token &token, Declarations &declarations, InstructionList &instructions, const Settings &settings)
	{
		const Token::Children &children = token.children();
		switch(token.type())
		{
		case Token::Nil:
			assert(children.empty());
			instructions.push_back(Instruction::push(token.sourceLocation(), Value::nil()));
			break;
		case Token::Root:
			assert(children.empty());
			// do nothing;
			break;
		case Token::List:
			{
				handleList(token, declarations, instructions, settings);
			}
			break;
		case Token::String:
			assert(children.empty());
			instructions.push_back(Instruction::push(token.sourceLocation(), Value::string(token.string())));
			break;
		case Token::Number:
			assert(children.empty());
			instructions.push_back(Instruction::push(token.sourceLocation(), Value::number(to<int>(token.string()))));
			break;
		case Token::Boolean:
			assert(children.empty());
			if (token.string() == "true")
			{
				instructions.push_back(Instruction::push(token.sourceLocation(), Value::boolean(true)));
			}
			else if (token.string() == "false")
			{
				instructions.push_back(Instruction::push(token.sourceLocation(), Value::boolean(false)));
			}
			else
			{
				throw CompilerBug("illegal boolean literal '" + token.string() + "' " + str(token.sourceLocation()));
			}
			break;
		case Token::Keyword:
			{
				throw ParseError(token.sourceLocation(), "Keyword '" + token.string() + "' must be first element of a list");
			}
			break;
		case Token::Identifier:
			{
				Identifier identifier = tryMakeIdentifier(token);
				switch(declarations.checkIdentifier(identifier))
				{
				case IDENTIFIER_DEFINITION_UNDEFINED:
					throw ParseError(token.sourceLocation(), "Variable '" + identifier.name() + "' not defined");
					break;
				case IDENTIFIER_DEFINITION_LOCAL:
					instructions.push_back(Instruction::refLocal(token.sourceLocation(), identifier));
					break;
				case IDENTIFIER_DEFINITION_CLOSURE:
					instructions.push_back(Instruction::refClosure(token.sourceLocation(), identifier));
					break;
				case IDENTIFIER_DEFINITION_GLOBAL:
					instructions.push_back(Instruction::refGlobal(token.sourceLocation(), identifier));
					break;
				default:
					throw CompilerBug("Failed to classify identifier " + identifier.name() + " at " + str(token.sourceLocation()));
				}

				for(Token::Children::const_iterator i = children.begin() ; i != children.end() ; ++i)
				{
					const Token &child = *i;
					if (child.type() != Token::Identifier)
					{
						throw CompilerBug("Expected identifier but got " + str(child.type()));
					}
					instructions.push_back(Instruction::memberAccess(token.sourceLocation(), child.string()));
				}
			}
			break;
		}
	}

	void indent(int num)
	{
		while(num --> 0)
		{
			std::cout << "| ";
		}
	}

	void printTree(const Token &token, int level = 0)
	{
		indent(level);
		const Token::Children &children = token.children();
		switch(token.type())
		{
		case Token::Nil:
			assert(children.empty());
			std::cout << "nil\n";
			break;
		case Token::Root:
			std::cout << "root {\n";
			for(Token::Children::const_iterator i = children.begin() ; i != children.end() ; ++i)
			{
				printTree(*i, level + 1);
			}
			indent(level);
			std::cout << "}\n";
			break;
		case Token::List:
			if (children.empty())
			{
				std::cout << "list <empty>\n";
			}
			else
			{
				std::cout << "list {\n";
				for(Token::Children::const_iterator i = children.begin() ; i != children.end() ; ++i)
				{
					printTree(*i, level + 1);
				}
				indent(level);
				std::cout << "}\n";
			}
			break;
		case Token::String:
			assert(children.empty());
			std::cout << "string: \"" << addEscapes(token.string()) << "\"\n";
			break;
		case Token::Number:
			assert(children.empty());
			std::cout << "number: " << token.string() << '\n';
			break;
		case Token::Boolean:
			assert(children.empty());
			std::cout << "boolean: " << token.string() << '\n';
			break;
		case Token::Keyword:
			assert(children.empty());
			std::cout << "keyword: " << token.string() << '\n';
			break;
		case Token::Identifier:
			std::cout << "identifier: " << token.string();
			for(Token::Children::const_iterator i = children.begin() ; i != children.end() ; ++i)
			{
				std::cout << '.' << i->string();
			}
			std::cout << '\n';
			break;
		}
	}
}

InstructionList parse(const Token &tree, Declarations &declarations, const Settings &settings)
{
	if (settings.printSyntaxTree)
	{
		printTree(tree);
	}

	InstructionList result;
	assert(tree.type() == Token::Root);
	const Token::Children &children = tree.children();
	for(Token::Children::const_iterator it = children.begin() ; it != children.end() ; ++it)
	{
		parse(*it, declarations, result, settings);
	}

	if (settings.printInstructions)
	{
		std::cout << "Parsing " << tree.sourceLocation() << '\n';
		printInstructions(result);
	}

	return result;
}
