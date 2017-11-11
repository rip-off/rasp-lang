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
#include "standard_library.h"

#define CURRENT_SOURCE_LOCATION SourceLocation(__FILE__, __LINE__)

namespace
{
	class AssertionError
	{
	public:
		AssertionError(const SourceLocation &sourceLocation, const std::string &message)
		:
			message_(message + " at " + str(sourceLocation))
		{
		}

		virtual const char *what() const
		{
			return message_.c_str();
		}
	private:
		std::string message_;
	};

	Token lex(const char *filename, int line, const std::string &source)
	{
		std::string fragmentName = "<unit test @ " + str(filename) + ":" + str(line) + ">";
		return ::lex(fragmentName, source);
	}
	#define lex(s) lex(__FILE__, __LINE__, s)

	int assertions = 0;

	int flushAssertions()
	{
		int previous = assertions;
		assertions = 0;
		return previous;
	}
/*
	void assertTrue(const SourceLocation &sourceLocation, bool expression, const std::string &message)
	{
		++assertions;
		if(!expression)
		{
			throw AssertionError(sourceLocation, message);
		}
	}
	#define assertTrue(EXPRESSION, MESSAGE) assertTrue(CURRENT_SOURCE_LOCATION, EXPRESSION, MESSAGE)
*/
	template <typename X, typename Y>
	void assertEquals(const SourceLocation &sourceLocation, const X &x, const Y &y)
	{
		++assertions;
		if (x != y)
		{
			throw AssertionError(sourceLocation, "'" + str(x) + "' should equal '" + str(y) + "'");
		}
	}
	#define assertEquals(X, Y) assertEquals(CURRENT_SOURCE_LOCATION, X, Y)

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

	void testLexerWithFunction()
	{
		std::string source = "(defun double (x) (* x 2))";
		Token token = lex(source);
		assertEquals(token.type(), Token::Root);
		
		const Token::Children &rootChildren = token.children();
		assertEquals(rootChildren.size(), 1);
		const Token &list = rootChildren.front();
		assertEquals(list.type(), Token::List);

		const Token::Children &children = list.children();
		assertEquals(children.size(), 4);

		assertEquals(children[0].type(), Token::Keyword);
		assertEquals(children[0].string(), "defun");

		assertEquals(children[1].type(), Token::Identifier);
		assertEquals(children[1].string(), "double");

		assertEquals(children[2].type(), Token::List);
		assertEquals(children[2].string(), "__list");

		const Token::Children &argumentList = children[2].children();
		assertEquals(argumentList.size(), 1);
		assertEquals(argumentList[0].type(), Token::Identifier);
		assertEquals(argumentList[0].string(), "x");

		assertEquals(children[3].type(), Token::List);
		assertEquals(children[3].string(), "__list");

		const Token::Children &functionBody = children[3].children();
		assertEquals(functionBody.size(), 3);
		assertEquals(functionBody[0].type(), Token::Identifier);
		assertEquals(functionBody[0].string(), "*");

		assertEquals(functionBody[1].type(), Token::Identifier);
		assertEquals(functionBody[1].string(), "x");

		assertEquals(functionBody[2].type(), Token::Number);
		assertEquals(functionBody[2].string(), "2");
	}

	void testLexerWithExplicitlyTypedFunction()
	{
		std::string source = "(defun double:number (x:number) (* x 2))";
		Token token = lex(source);
		assertEquals(token.type(), Token::Root);

		const Token::Children &rootChildren = token.children();
		assertEquals(rootChildren.size(), 1);
		const Token &list = rootChildren.front();
		assertEquals(list.type(), Token::List);

		const Token::Children &children = list.children();
		assertEquals(children.size(), 4);

		assertEquals(children[0].type(), Token::Keyword);
		assertEquals(children[0].string(), "defun");

		assertEquals(children[1].type(), Token::Declaration);
		assertEquals(children[1].string(), "__declaration");

		const Token::Children &functionNameDeclaration = children[1].children();
		assertEquals(functionNameDeclaration.size(), 2);
		assertEquals(functionNameDeclaration[0].type(), Token::Identifier);
		assertEquals(functionNameDeclaration[0].string(), "double");

		assertEquals(functionNameDeclaration[1].type(), Token::Identifier);
		assertEquals(functionNameDeclaration[1].string(), "number");

		assertEquals(children[2].type(), Token::List);
		assertEquals(children[2].string(), "__list");

		const Token::Children &argumentList = children[2].children();
		assertEquals(argumentList.size(), 1);
		assertEquals(argumentList[0].type(), Token::Declaration);
		assertEquals(argumentList[0].string(), "__declaration");

		const Token::Children &argumentDeclaration = argumentList[0].children();
		assertEquals(argumentDeclaration.size(), 2);
		assertEquals(argumentDeclaration[0].type(), Token::Identifier);
		assertEquals(argumentDeclaration[0].string(), "x");

		assertEquals(argumentDeclaration[1].type(), Token::Identifier);
		assertEquals(argumentDeclaration[1].string(), "number");

		assertEquals(children[3].type(), Token::List);
		assertEquals(children[3].string(), "__list");

		const Token::Children &functionBody = children[3].children();
		assertEquals(functionBody.size(), 3);
		assertEquals(functionBody[0].type(), Token::Identifier);
		assertEquals(functionBody[0].string(), "*");

		assertEquals(functionBody[1].type(), Token::Identifier);
		assertEquals(functionBody[1].string(), "x");

		assertEquals(functionBody[2].type(), Token::Number);
		assertEquals(functionBody[2].string(), "2");
	}
}

namespace
{
	typedef void BasicUnitTest();

	int runUnitTest(const std::string &name, BasicUnitTest *unitTest)
	{
		try
		{
			std::cout << "Running " << name << "..." << '\n';
			unitTest();
			std::cout << "PASSED " << flushAssertions() << " assertions\n";
			return 0;
		}
		catch(const RaspError &e)
		{
			std::cerr << "ERROR: " << e.what() << '\n';
			return 1;
		}
		catch(const AssertionError &e)
		{
			std::cerr << "FAILED: " << e.what() << '\n';
			return 1;
		}
		catch(...)
		{
			std::cerr << "Unexpected FATAL error" << std::endl;
			throw;
		}
	}
}

#define RUN_BASIC_TEST(testName) runUnitTest(#testName, &testName)

int runLexerUnitTests()
{
	return 0
	+ RUN_BASIC_TEST(testLexer)
	+ RUN_BASIC_TEST(testLexerWithFunction)
	+ RUN_BASIC_TEST(testLexerWithExplicitlyTypedFunction)
	;
}

