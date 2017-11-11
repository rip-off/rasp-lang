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

	void assertTrue(const SourceLocation &sourceLocation, bool expression, const std::string &message)
	{
		++assertions;
		if(!expression)
		{
			throw AssertionError(sourceLocation, message);
		}
	}
	#define assertTrue(EXPRESSION, MESSAGE) assertTrue(CURRENT_SOURCE_LOCATION, EXPRESSION, MESSAGE)

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

	void testInterpreter(Interpreter &interpreter)
	{
		InstructionList instructions;
		instructions.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, Value::number(42)));
		instructions.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, Value::number(13)));
		instructions.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, Value::number(16)));
		const Value *value = interpreter.global(Identifier("+"));
		assertTrue(value, "Expected there is a global for '+'");
		instructions.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, *value));
		instructions.push_back(Instruction::call(CURRENT_SOURCE_LOCATION, 3));
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

	void testParser(Interpreter &interpreter)
	{
		SourceLocation sourceLocation = CURRENT_SOURCE_LOCATION;
		Token function = Token::identifier(sourceLocation, "+");
		Token left = Token::number(sourceLocation, "42");
		Token right = Token::number(sourceLocation, "13");
		Token list = Token::list(sourceLocation);
		list.addChild(function);
		list.addChild(left);
		list.addChild(right);
		Token root = Token::root(sourceLocation);
		root.addChild(list);
		Declarations declarations = interpreter.declarations();
		InstructionList result = parse(root, declarations, interpreter.settings());
		assertEquals(result.size(), 4);

		assertEquals(result[0].type(), Instruction::Push);
		assertEquals(result[0].value().number(), 13);

		assertEquals(result[1].type(), Instruction::Push);
		assertEquals(result[1].value().number(), 42);

		assertEquals(result[2].type(), Instruction::RefGlobal);
		assertEquals(result[2].value().string(), "+");

		assertEquals(result[3].type(), Instruction::Call);
		assertEquals(result[3].value().number(), 2);
	}

	void testAll(Interpreter &interpreter)
	{
		std::string source = "(+ (* 2 42) (/ 133 10) (- 1 6))";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 84 + 13 - 5);
	}
	
	void testVariablesInGlobalScope(Interpreter &interpreter)
	{
		std::stringstream source;
        source << "(var global 1)";
        source << "(set global (+ global 1))";
        source << "global";
		Token token = lex(source.str());
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 2);
	}

	void testGlobalsReferencesInFunction(Interpreter &interpreter)
	{
		std::stringstream source;
        source << "(var global 1)";
        source << "(defun incrementGlobal () (set global (+ global 1)))";
        source << "(incrementGlobal)";
        source << "global";
		Token token = lex(source.str());
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 2);
	}
	
	void testLocalsInFunction(Interpreter &interpreter)
	{
		std::stringstream source;
        source << "(defun incrementLocal ()";
        source << "  (var local 1)";
        source << "  (set local (+ local 1))";
        source << "  local)";
        source << "(incrementLocal)";
		Token token = lex(source.str());
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 2);
	}
	
	void testClosureCanAccessVariableInOuterScope(Interpreter &interpreter)
	{
		std::stringstream source;
        source << "(defun outer ()";
        source << "  (var capture 42)";
        source << "  (defun inner () capture)";
        source << "  (inner))";
        source << "(outer)";
		Token token = lex(source.str());
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

// TODO: test closure variable access
#if 0
	void testClosureSeesUpdatedVariableInOuterScope(Interpreter &interpreter)
	{
		std::stringstream source;
        source << "(defun outer ()";
        source << "  (var capture 42)";
        source << "  (defun inner () capture)";
        source << "  (set capture 13)";
        source << "  (inner))";
        source << "(outer)";
		Token token = lex(source.str());
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 13);
	}

	void testClosureCanModifyVariableInOuterScope(Interpreter &interpreter)
	{
		std::stringstream source;
        source << "(defun outer ()";
        source << "  (var capture 1)";
        source << "  (defun inner () (set capture (+ capture 1)))";
        source << "  (inner)";
        source << "  capture)";
        source << "(outer)";
		Token token = lex(source.str());
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 2);
	}
#endif

	void testReturnedClosureCanStillAccessVariableInOuterScope(Interpreter &interpreter)
	{
		std::stringstream source;
        source << "(defun outer ()";
        source << "  (var capture 13)";
        source << "  (defun inner () capture)";
        source << "  inner)";
        source << "(var closure (outer))";
        source << "(closure)";
		Token token = lex(source.str());
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 13);
	}

	void testClosure(Interpreter &interpreter)
	{
		InstructionList instructions;
		instructions.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, Value::number(42)));
		instructions.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, Value::number(13)));
		const Value *value = interpreter.global(Identifier("+"));
		assertTrue(value, "Expected there is a global for '+'");
		instructions.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, *value));
		int argumentsToCapture = 2;
		instructions.push_back(Instruction::capture(CURRENT_SOURCE_LOCATION, argumentsToCapture));
		instructions.push_back(Instruction::call(CURRENT_SOURCE_LOCATION, 0));
		Value result = interpreter.exec(instructions);
		assertEquals(Value::TNumber, result.type());
		assertEquals(result.number(), 55);
	}

	void testTypesAndMemberAccess(Interpreter &interpreter)
	{
		std::stringstream source;
        source << "(type Person id name)";
        source << "(var alice (new Person 13 \"Alice\"))";
        source << "(var bob (new Person 42 \"Bob\"))";
		source << "(+ \"People: \" alice.name \", \" bob.name)";
		Token token = lex(source.str());
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TString);
		assertEquals(result.string(), "People: Alice, Bob");
	}

	void testSimpleLoop(Interpreter &interpreter)
	{
		std::stringstream source;
        source << "(var result 2)";
        source << "(while (< result 100)";
        source << "  (set result (* result 2))";
		source << ")";
		source << "result";
		Token token = lex(source.str());
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 128);
	}
}

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

typedef void InterpreterTest(Interpreter &);

int runUnitTest(const std::string &name, InterpreterTest *unitTest, const Settings &settings)
{
	try
	{
		std::cout << "Running " << name << "..." << '\n';
		Interpreter::Globals globals = standardLibrary();
		Interpreter interpreter(globals, settings);
		unitTest(interpreter);
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

#define RUN_BASIC_TEST(testName) runUnitTest(#testName, &testName)
#define RUN_INTERPRETER_TEST(testName, interpreter) runUnitTest(#testName, &testName, interpreter)

int runUnitTests(const Settings &settings)
{
	return 0
	+ RUN_BASIC_TEST(testLexer)
	+ RUN_BASIC_TEST(testLexerWithFunction)
	+ RUN_BASIC_TEST(testLexerWithExplicitlyTypedFunction)
	+ RUN_INTERPRETER_TEST(testParser, settings)
	+ RUN_INTERPRETER_TEST(testInterpreter, settings)
	+ RUN_INTERPRETER_TEST(testAll, settings)
	+ RUN_INTERPRETER_TEST(testVariablesInGlobalScope, settings)
	+ RUN_INTERPRETER_TEST(testGlobalsReferencesInFunction, settings)
	+ RUN_INTERPRETER_TEST(testLocalsInFunction, settings)
	+ RUN_INTERPRETER_TEST(testClosureCanAccessVariableInOuterScope, settings)
	/*
	+ RUN_INTERPRETER_TEST(testClosureSeesUpdatedVariableInOuterScope, settings)
	+ RUN_INTERPRETER_TEST(testClosureCanModifyVariableInOuterScope, settings)
	*/
	+ RUN_INTERPRETER_TEST(testReturnedClosureCanStillAccessVariableInOuterScope, settings)
	+ RUN_INTERPRETER_TEST(testClosure, settings)
	+ RUN_INTERPRETER_TEST(testTypesAndMemberAccess, settings)
	+ RUN_INTERPRETER_TEST(testSimpleLoop, settings)
	;
}

