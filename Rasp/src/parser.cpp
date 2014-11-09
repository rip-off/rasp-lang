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
	bool isDefined(const std::vector<Identifier> &declarations, const Identifier identifier)
	{
		return std::find(declarations.begin(), declarations.end(), identifier) != declarations.end();
	}

	void parse(const Token &token, std::vector<Identifier> &declarations, InstructionList &list)
	{
		const Token::Children &children = token.children();
		switch(token.type())
		{
		case Token::Nil:
			assert(children.empty());
			list.push_back(Instruction::push(Value::nil()));
			break;
		case Token::Root:
			assert(children.empty());
			// do nothing;
			break;
		case Token::List:
			{
				if(children.empty())
				{
					// TODO: investigate this, compile time error?
					list.push_back(Instruction::push(Value::nil()));
				}
				else
				{
					Token::Type firstTokenType = children.front().type();
					if(firstTokenType == Token::Loop)
					{
						if(children.size() == 1)
						{
							throw ParseError(token.line(), "'while' expression is missing condition");
						}
						else if(children.size() == 2)
						{
							throw ParseError(token.line(), "'while' expression is missing code to execute");
						}
						unsigned previousInstructionCount = list.size();
						// Evaluate the conditional expression first
						parse(children[1], declarations, list);
						unsigned conditionExpressionInstructions = list.size() - previousInstructionCount;
						// Generate the list of instructions to be executed if branch is taken
						InstructionList tempInstructions;
						for(unsigned i = 2 ; i < children.size() ; ++i)
						{
							parse(children[i], declarations, tempInstructions);
						}
						unsigned bodyInstructions = tempInstructions.size();
						// Actual branch instruction
						// +1 for the loop instruction itself!
						list.push_back(Instruction::jump(bodyInstructions + 1));
						// Insert the remaining instructions into the stream
						list.insert(list.end(), tempInstructions.begin(), tempInstructions.end());
						// Return to loop start
						// +1 for jump instruction
						// +1 for this loop instruction itself!
						list.push_back(Instruction::loop(bodyInstructions + 1 + conditionExpressionInstructions + 1));
					}
					else if(firstTokenType == Token::Condition)
					{
						if(children.size() == 1)
						{
							throw ParseError(token.line(), "Conditional expression is missing condition");
						}
						else if(children.size() == 2)
						{
							throw ParseError(token.line(), "Conditional expression is missing code to execute");
						}
						// Evaluate the conditional expression first
						parse(children[1], declarations, list);
						// Generate the list of instructions to be executed if branch is taken
						InstructionList tempInstructions;
						for(unsigned i = 2 ; i < children.size() ; ++i)
						{
							parse(children[i], declarations, tempInstructions);
						}
						// Actual branch instruction
						list.push_back(Instruction::jump(tempInstructions.size()));
						// Insert the remaining instructions into the stream
						list.insert(list.end(), tempInstructions.begin(), tempInstructions.end());
					}
					else if(firstTokenType == Token::VariableDeclaration)
					{
						if(children.size() == 2)
						{
							throw ParseError(token.line(), "Variable declaration requires a name");
						}
						else if(children.size() != 3)
						{
							throw ParseError(token.line(), "Missing initialisation value");
						}
						Identifier identifier = Identifier(children[1].string());
						if (isDefined(declarations, identifier))
						{
							throw ParseError(token.line(), "Identity " + identifier.name() + " already defined");
						}
						declarations.push_back(identifier);				
						parse(children[2], declarations, list);
						list.push_back(Instruction::assign(identifier.name()));
					}
					else if(firstTokenType == Token::Assignment)
					{
						if(children.size() == 2)
						{
							throw ParseError(token.line(), "Variable assignment requires a name");
						}
						else if(children.size() != 3)
						{
							throw ParseError(token.line(), "Missing assignment value");
						}
						Identifier identifier = Identifier(children[1].string());
						if (!isDefined(declarations, identifier))
						{
							throw ParseError(token.line(), "Identifier '" + identifier.name() + "' not defined");
						}	
						parse(children[2], declarations, list);
						list.push_back(Instruction::assign(identifier.name()));
					}
					else if(firstTokenType == Token::FunctionDeclaration)
					{
						if(children.size() == 2)
						{
							throw ParseError(token.line(), "Function requires a name");
						}
						/* TODO: else if(children.size() == 3)
						{
							throw ParseError(token.line(), "Function requires parameter lists");
						} */
						else if(children.size() < /* TODO */ 3)
						{
							throw ParseError(token.line(), "Function lacks a body");
						}
						Identifier identifier = Identifier(children[1].string());
						if (isDefined(declarations, identifier))
						{
							throw ParseError(token.line(), "Identifier " + identifier.name() + " already defined");
						}
						declarations.push_back(identifier);			

						InstructionList tempInstructions;
						for(unsigned i = 2 ; i < children.size() ; ++i)
						{
							parse(children[i], declarations, tempInstructions);
						}
						InternalFunction function(identifier.name(), tempInstructions);
						list.push_back(Instruction::push(function));
						list.push_back(Instruction::assign(identifier.name()));
					}
					else
					{
						for(Token::Children::const_reverse_iterator i = children.rbegin() ; i != children.rend() ; ++i)
						{
							parse(*i, declarations, list);
						}
						// Call expects the number of arguments, so we must omit 1 element
						// This is because the function is the mandatory first element
						// TODO: check the first element is a function at compile time
						list.push_back(Instruction::call(children.size() - 1));
					}
				}
			}
			break;
		case Token::String:
			assert(children.empty());
			list.push_back(Instruction::push(token.string()));
			break;
		case Token::Loop:
			{
				throw ParseError(token.line(), "'while' must be first element of a list");
			}
			break;
		case Token::Number:
			assert(children.empty());
			list.push_back(Instruction::push(to<int>(token.string())));
			break;
		case Token::Condition:
			{
				throw ParseError(token.line(), "'if' must be first element of a list");
			}
			break;
		case Token::Assignment:
			{
				throw ParseError(token.line(), "'set' must be first element of a list");
			}
			break;
		case Token::VariableDeclaration:
			{
				throw ParseError(token.line(), "'var' must be first element of a list");
			}
			break;
		case Token::FunctionDeclaration:
			{
				throw ParseError(token.line(), "'defun' must be first element of a list");
			}
			break;
		case Token::Identifier:
			{
				assert(children.empty());
				Identifier identifier = Identifier(token.string());
				if (!isDefined(declarations, identifier))
				{
					throw ParseError(token.line(), "Variable '" + identifier.name() + "' not defined");
				}
				list.push_back(Instruction::ref(identifier));
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
			std::cout << "nil";
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
		case Token::Loop:
			assert(children.empty());
			std::cout << "While";
			break;
		case Token::String:
			assert(children.empty());
			std::cout << "String(" << token.string() << ')' << '\n';
			break;
		case Token::Number:
			assert(children.empty());
			std::cout << "Number(" << token.string() << ')' << '\n';
			break;
		case Token::Condition:
			assert(children.empty());
			std::cout << "If";
			break;
		case Token::VariableDeclaration:
			assert(children.empty());
			std::cout << "Var";
			break;
		case Token::FunctionDeclaration:
			assert(children.empty());
			std::cout << "Defun";
			break;
		case Token::Assignment:
			assert(children.empty());
			std::cout << "Assignment(" << token.string() << ')' << '\n';
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
