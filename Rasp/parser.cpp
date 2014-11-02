#include "parser.h"

#include <iostream>

#include "token.h"
#include "bindings.h"
#include "settings.h"
#include "exceptions.h"

namespace
{
	void parse(const Token &token, const Bindings &bindings, InstructionList &list)
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
					if(children.front().type() == Token::Condition)
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
						parse(children[1], bindings, list);
						// Generate the list of instructions to be executed if branch is taken
						InstructionList tempInstructions;
						for(unsigned i = 2 ; i < children.size() ; ++i)
						{
							parse(children[i], bindings, tempInstructions);
						}
						// Actual branch instruction
						list.push_back(Instruction::jump(tempInstructions.size()));
						// Insert the remaining instructions into the stream
						list.insert(list.end(), tempInstructions.begin(), tempInstructions.end());
					}
					else
					{
						for(Token::Children::const_reverse_iterator i = children.rbegin() ; i != children.rend() ; ++i)
						{
							parse(*i, bindings, list);
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
		case Token::Number:
			assert(children.empty());
			list.push_back(Instruction::push(to<int>(token.string())));
			break;
		case Token::Condition:
			{
#if 0
				if(children.empty())
				{
					throw ParseError(token.line(), "Conditional expression is missing condition");
				}
				if(children.size() < 2) 
				{
					throw ParseError(token.line(), "Conditional expression is missing code to execute");
				}
#endif
				//list.push_back(Instruction::condition(children.size() - 1));
				throw ParseError(token.line(), "If must be first element of a list");

				
				// TODO:
#if 0
				
				InstructionList condition;
				parse(children.front(), bindings, condition);
				InstructionList conditionalInstructions;
				for(Token::Children::const_reverse_iterator i = children.rbegin() ; i != children.rend() - 1 ; ++i)
				{
					parse(*i, bindings, conditionalInstructions);
				}
				list.push_back(Instruction::condition(condition, conditionalInstructions));
#endif
			}
			break;
		case Token::Identifier:
			{
				assert(children.empty());
				const Value *value = tryFind(bindings, token.string());
				if(!value)
				{
					throw ParseError(token.line(), "Unknown binding " + token.string());
				}
				list.push_back(Instruction::push(*value));
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
			for(Token::Children::const_reverse_iterator i = children.rbegin() ; i != children.rend() ; ++i)
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
		case Token::Condition:
			std::cout << "If {\n";
			for(Token::Children::const_reverse_iterator i = children.rbegin() ; i != children.rend() ; ++i)
			{
				printTree(*i, level + 1);
			}
			printTabs(level);
			std::cout << "}";
			break;
		case Token::Identifier:
			assert(children.empty());
			std::cout << "Identifier(" << token.string() << ')' << '\n';
			break;
		}
		std::cout << '\n';
	}
}

InstructionList parse(const Token &tree, const Bindings &bindings, const Settings &settings)
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
		parse(*it, bindings, result);
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
