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
#include "standard_math.h"
#include "standard_library.h"

#define RASP_ENABLE_ASSERTION_MACROS
#include "assert.h"

struct Source
{
	Source()
	{
	}

	Source(const char *content)
	{
		stream << content;
	}

	std::string str() const
	{
		return stream.str();
	}

	std::stringstream stream;
};

Source &operator<<(Source &source, const std::string line)
{
	source.stream << line << '\n';
	return source;
}

namespace
{
	Token lex(const char *filename, int line, const Source &source)
	{
		std::string fragmentName = "<unit test @ " + str(filename) + ":" + str(line) + ">";
		return ::lex(fragmentName, source.str());
	}
	#define lex(s) lex(__FILE__, __LINE__, s)

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

	void testParser(Interpreter &interpreter)
	{
		SourceLocation sourceLocation = CURRENT_SOURCE_LOCATION;
		Token function = Token::identifier(sourceLocation, Identifier("+"));
		Token left = Token::number(sourceLocation, "42");
		Token right = Token::number(sourceLocation, "13");
		Token list = Token::list(sourceLocation);
		list.addChild(function);
		list.addChild(left);
		list.addChild(right);
		Token root = Token::list(sourceLocation);
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
		Source source = "(+ (* 2 42) (/ 133 10) (- 1 6))";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 84 + 13 - 5);
	}
	
	void testVariablesInGlobalScope(Interpreter &interpreter)
	{
		Source source;
        source << "(var global 1)";
        source << "(set global (+ global 1))";
        source << "global";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 2);
	}

	void testGlobalsReferencesInFunction(Interpreter &interpreter)
	{
		Source source;
        source << "(var global 1)";
        source << "(defun incrementGlobal () (set global (+ global 1)))";
        source << "(incrementGlobal)";
        source << "global";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 2);
	}
	
	void testLocalsInFunction(Interpreter &interpreter)
	{
		Source source;
        source << "(defun incrementLocal ()";
        source << "  (var local 1)";
        source << "  (set local (+ local 1))";
        source << "  local)";
        source << "(incrementLocal)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 2);
	}
	
	void testClosureCanAccessVariableInOuterScope(Interpreter &interpreter)
	{
		Source source;
        source << "(defun outer ()";
        source << "  (var capture 42)";
        source << "  (defun inner () capture)";
        source << "  (inner))";
        source << "(outer)";
		Token token = lex(source);
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
		Source source;
        source << "(defun outer ()";
        source << "  (var capture 42)";
        source << "  (defun inner () capture)";
        source << "  (set capture 13)";
        source << "  (inner))";
        source << "(outer)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 13);
	}

	void testClosureCanModifyVariableInOuterScope(Interpreter &interpreter)
	{
		Source source;
        source << "(defun outer ()";
        source << "  (var capture 1)";
        source << "  (defun inner () (set capture (+ capture 1)))";
        source << "  (inner)";
        source << "  capture)";
        source << "(outer)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 2);
	}
#endif

	void testReturnedClosureCanStillAccessVariableInOuterScope(Interpreter &interpreter)
	{
		Source source;
        source << "(defun outer ()";
        source << "  (var capture 13)";
        source << "  (defun inner () capture)";
        source << "  inner)";
        source << "(var closure (outer))";
        source << "(closure)";
		Token token = lex(source);
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
		Source source;
        source << "(type Person id name)";
        source << "(var alice (new Person 13 \"Alice\"))";
        source << "(var bob (new Person 42 \"Bob\"))";
		source << "(concat \"People: \" alice.name \", \" bob.name)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TString);
		assertEquals(result.string(), "People: Alice, Bob");
	}

	void testSimpleLoop(Interpreter &interpreter)
	{
		Source source;
        source << "(var result 2)";
        source << "(while (< result 100)";
        source << "  (set result (* result 2))";
		source << ")";
		source << "result";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 128);
	}

	void testComplexLoop(Interpreter &interpreter)
	{
		Source source;
        source << "(var result 0)";
        source << "(var i 0)";
        source << "(while (< i 10)";
        source << "  (set result (+ result i))";
        source << "  (set i (+ i 1))";
		source << ")";
		source << "result";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 45);
	}

	void testLoopWithInnerVariableDeclaration(Interpreter &interpreter)
	{
		Source source;
        source << "(var result 0)";
        source << "(var i 0)";
        source << "(while (< i 10)";
		source << "  (var expression (+ result i))";
        source << "  (set result expression)";
        source << "  (set i (+ i 1))";
		source << ")";
		source << "result";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 45);
	}

	void testVariableDeclarationWithNumberType(Interpreter &interpreter)
	{
		Source source;
		source << "(var result:number 13)";
		source << "result";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 13);
	}

	void testVariableDeclarationWithStringType(Interpreter &interpreter)
	{
		Source source;
		source << "(var result:string \"Hello, World\")";
		source << "result";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TString);
		assertEquals(result.string(), "Hello, World");
	}

	void testVariableDeclarationWithBooleanType(Interpreter &interpreter)
	{
		Source source;
		source << "(var result:boolean true)";
		source << "result";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TBoolean);
		assertEquals(result.boolean(), true);
	}

	void testFunctionDeclarationsWithTypes(Interpreter &interpreter)
	{
		Source source;
		source << "(defun double:number (x:number) (* x 2))";
		source << "(var result (double 42))";
		source << "result";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 84);
	}

	void testTypeDeclarationWithMemberTypes(Interpreter &interpreter)
	{
		Source source;
		source << "(type Person id:number name:string)";
		source << "(var alice (new Person 42 \"Alice\"))";
		source << "alice.name";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TString);
		assertEquals(result.string(), "Alice");
	}

	void testFunctionRecursion(Interpreter &interpreter)
	{
		Source source;
		source << "(defun recurse (n)";
		source << "  (if (<= n 0) 0)";
		source << "  (if (> n 0)";
		source << "    (+ n (recurse (- n 1)))";
		source << "  )";
		source << ")";
		source << "(recurse 10)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 55);
	}

	void testStringConcatenation(Interpreter &interpreter)
	{
		Source source = "(concat \"number: \" 42 \", boolean: \" true)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		Value result = interpreter.exec(instructions);
		assertEquals(result.type(), Value::TString);
		assertEquals(result.string(), "number: 42, boolean: true");
	}

	void testReferenceToUndefinedVariable(Interpreter &interpreter)
	{
		Source source = "undefinedVariable";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Variable 'undefinedVariable' not defined");
		}
	}

	void testVariableDeclarationWithUndefinedType(Interpreter &interpreter)
	{
		Source source = "(var x:UndefinedType 42)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Unknown type 'UndefinedType'");
		}
	}
}

namespace
{
	typedef void InterpreterTest(Interpreter &);

	int runUnitTest(const std::string &name, InterpreterTest *unitTest, const Settings &settings)
	{
		try
		{
			std::cout << "Running " << name << "..." << '\n';
			Interpreter::Globals globals;
			standardMath(globals);
			standardLibrary(globals);
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
}

#define RUN_INTERPRETER_TEST(testName, interpreter) runUnitTest(#testName, &testName, interpreter)

int runUnitTests(const Settings &settings)
{
	return 0
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
	+ RUN_INTERPRETER_TEST(testComplexLoop, settings)
	+ RUN_INTERPRETER_TEST(testLoopWithInnerVariableDeclaration, settings)
	+ RUN_INTERPRETER_TEST(testVariableDeclarationWithNumberType, settings)
	+ RUN_INTERPRETER_TEST(testVariableDeclarationWithStringType, settings)
	+ RUN_INTERPRETER_TEST(testVariableDeclarationWithBooleanType, settings)
	+ RUN_INTERPRETER_TEST(testFunctionDeclarationsWithTypes, settings)
	+ RUN_INTERPRETER_TEST(testTypeDeclarationWithMemberTypes, settings)
	+ RUN_INTERPRETER_TEST(testFunctionRecursion, settings)
	+ RUN_INTERPRETER_TEST(testStringConcatenation, settings)
	+ RUN_INTERPRETER_TEST(testReferenceToUndefinedVariable, settings)
	+ RUN_INTERPRETER_TEST(testVariableDeclarationWithUndefinedType, settings)
	;
}

