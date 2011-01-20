#include "parser.h"

// TODO: remove me...
#include <iostream>

#include "token.h"
#include "bindings.h"
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
			break;
		case Token::Number:
			assert(children.empty());
			list.push_back(Instruction::push(to<int>(token.string())));
			break;
		case Token::Identifier:
			{
				assert(children.empty());
				const Value *value = tryFind(bindings, token.string());
				if(!value)
				{
					throw ParseError("Unknown binding " + token.string());
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
		case Token::Number:
			assert(children.empty());
			std::cout << "Number(" << token.string() << ')' << '\n';
			break;
		case Token::Identifier:
			assert(children.empty());
			std::cout << "Identifier(" << token.string() << ')' << '\n';
			break;
		}
		std::cout << '\n';
	}
}

InstructionList parse(const Token &tree, const Bindings &bindings)
{
#if 0
	printTree(tree,1);
#endif

	InstructionList result;
	assert(tree.type() == Token::Root);
	const Token::Children &children = tree.children();
	for(Token::Children::const_iterator i = children.begin() ; i != children.end() ; ++i)
	{
		parse(*i, bindings, result);
	}
	return result;
}