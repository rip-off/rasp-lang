#include "unit_tests.h"

#include <cassert>
#include <iostream>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "exceptions.h"
#include "instruction.h"
#include "interpreter.h"

namespace
{
	void testInterpreter(Interpreter &interpreter)
	{
		InstructionList instructions;
		instructions.push_back(Instruction::push(42));
		instructions.push_back(Instruction::push(13));
		instructions.push_back(Instruction::push(16));
		const Value *value = interpreter.binding("+");
		assert(value);
		instructions.push_back(Instruction::push(*value));
		instructions.push_back(Instruction::call(3));
		Value result = interpreter.exec(instructions);
		assert(result.isNumber());
		assert(result.number() == (42 + 13 + 16));
	}

	void testLexer()
	{
		std::string source = "(+ 13 42)";
		Token token = lex(source);
		assert(token.type() == Token::Root);
		
		const Token::Children &rootChildren = token.children();
		assert(rootChildren.size() == 1);
		const Token &list = rootChildren.front();
		assert(list.type() == Token::List);

		const Token::Children &children = list.children();
		assert(children.size() == 3);
		assert(children[0].type() == Token::Identifier && children[0].string() == "+");
		assert(children[1].type() == Token::Number && children[1].string() == "13");
		assert(children[2].type() == Token::Number && children[2].string() == "42");
	}

	void testParser(Interpreter &interpreter)
	{
		Token function = Token::identifier("+");
		Token left = Token::number("42");
		Token right = Token::number("13");
		Token list = Token::list();
		list.addChild(function);
		list.addChild(left);
		list.addChild(right);
		Token root;
		root.addChild(list);
		InstructionList result = parse(root, interpreter.bindings());
		assert(result.size() == 4);
		assert(result[0].type() == Instruction::Push && result[0].value().number() == 13);
		assert(result[1].type() == Instruction::Push && result[1].value().number() == 42);
		assert(result[2].type() == Instruction::Push && result[2].value().isFunction() && result[2].value().function().name() == "+");
		assert(result[3].type() == Instruction::Call && result[3].value().number() == 2);
	}

	void testAll(Interpreter &interpreter)
	{
		std::string source = "(+ (* 2 42) (/ 133 10) (- 1 6))";
		Token token = lex(source);
		InstructionList instructions = parse(token, interpreter.bindings());
		Value result = interpreter.exec(instructions);
		assert(result.isNumber());
		assert(result.number() == 84 + 13 - 5);
	}
}

void runUnitTests(Interpreter &interpreter)
{
	try
	{
		testLexer();
		testParser(interpreter);
		testInterpreter(interpreter);

		testAll(interpreter);
	}
	catch(const RaspError &e)
	{
		std::cerr << "Exception in unit tests: " << e.what() << '\n';
		assert(false);
	}
}
