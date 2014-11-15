#include "unit_tests.h"

#include <cassert>
#include <iostream>

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "settings.h"
#include "exceptions.h"
#include "instruction.h"
#include "interpreter.h"

namespace
{
	class AssertionError
	{
	public:
		AssertionError(int line, const std::string &message) 
		:
			message_(" at " + str(line) + " " + message)
		{
		}

		virtual const char *what() const
		{
			return message_.c_str();
		}
	private:
		std::string message_;
	};

	void assertTrue(int line, bool expression, const std::string &message)
	{
		if(!expression)
		{
			throw AssertionError(line, message);
		}
	}
	#define assertTrue(EXPRESSION, MESSAGE) assertTrue(__LINE__, EXPRESSION, MESSAGE)

	template <typename X, typename Y>
	void assertEquals(int line, const X &x, const Y &y)
	{
		if (x != y)
		{
			throw AssertionError(line, "'" + str(x) + "' should equal '" + str(y) + "'");
		}
	}
	#define assertEquals(X, Y) assertEquals(__LINE__, X, Y)

	InstructionList parse(const Token &token, std::vector<Identifier> &declarations)
	{
		Settings settings;
		return parse(token, declarations, settings);
	}

	void testInterpreter(Interpreter &interpreter)
	{
		InstructionList instructions;
		instructions.push_back(Instruction::push(42));
		instructions.push_back(Instruction::push(13));
		instructions.push_back(Instruction::push(16));
		const Value *value = interpreter.binding(Identifier("+"));
		assertTrue(value, "Expected there is a binding for '+'");
		instructions.push_back(Instruction::push(*value));
		instructions.push_back(Instruction::call(3));
		Value result = interpreter.exec(instructions);
		assertEquals(Value::TNumber, result.type());
		assertEquals(result.number(), (42 + 13 + 16));
	}

	void testLexer()
	{
		std::string source = "(+ 13 42)";
		Token token = lex(source);
		assertEquals(token.type(), Token::Root);
		
		const Token::Children &rootChildren = token.children();
		assertEquals(rootChildren.size(), 1);
		const Token &list = rootChildren.front();
		assertEquals(list.type(), Token::List);

		const Token::Children &children = list.children();
		assertEquals(children.size(), 3);

		assertEquals(children[0].type(), Token::Identifier);
		assertEquals(children[0].string(), "+");

		assertEquals(children[1].type(), Token::Number);
		assertEquals(children[1].string(), "13");

		assertEquals(children[2].type(), Token::Number);
		assertEquals(children[2].string(), "42");
	}

	void testParser(Interpreter &interpreter)
	{
		unsigned line = 1;
		Token function = Token::identifier(line, "+");
		Token left = Token::number(line, "42");
		Token right = Token::number(line, "13");
		Token list = Token::list(line);
		list.addChild(function);
		list.addChild(left);
		list.addChild(right);
		Token root = Token::root(line);
		root.addChild(list);
		std::vector<Identifier> declarations = interpreter.declarations();
		InstructionList result = parse(root, declarations);
		assertEquals(result.size(), 4);

		assertEquals(result[0].type(), Instruction::Push);
		assertEquals(result[0].value().number(), 13);

		assertEquals(result[1].type(), Instruction::Push);
		assertEquals(result[1].value().number(), 42);

		assertEquals(result[2].type(), Instruction::Ref);
		assertEquals(result[2].value().string(), "+");

		assertEquals(result[3].type(), Instruction::Call);
		assertEquals(result[3].value().number(), 2);
	}

	void testAll(Interpreter &interpreter)
	{
		std::string source = "(+ (* 2 42) (/ 133 10) (- 1 6))";
		Token token = lex(source);
		std::vector<Identifier> declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations);
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 84 + 13 - 5);
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
	}
	catch(const AssertionError &e)
	{
		std::cerr << "Assertion error in unit tests " << e.what() << '\n';
	}
}
