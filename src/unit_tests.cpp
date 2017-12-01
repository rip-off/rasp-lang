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
#include "internal_function.h"
#include "standard_library_error.h"

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

	Source(const std::string &content)
	{
		stream << content;
	}

	Source(const Source &source)
	{
		stream << source.str();
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

	Value execute(const char *filename, int line, Interpreter &interpreter, const Source &source)
	{
		Token token = lex(filename, line, source);
		Declarations declarations = interpreter.declarations();
		InstructionList instructions = parse(token, declarations, interpreter.settings());
		return interpreter.exec(instructions);

	}
	#define lex(source) lex(__FILE__, __LINE__, source)
	#define execute(interpreter, source) execute(__FILE__, __LINE__, interpreter, source)

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

		assertEquals(result[0].type(), Instruction::PUSH);
		assertEquals(result[0].value().number(), 13);

		assertEquals(result[1].type(), Instruction::PUSH);
		assertEquals(result[1].value().number(), 42);

		assertEquals(result[2].type(), Instruction::REF_GLOBAL);
		assertEquals(result[2].value().string(), "+");

		assertEquals(result[3].type(), Instruction::CALL);
		assertEquals(result[3].value().number(), 2);
	}

	void testMathExpression(Interpreter &interpreter)
	{
		Source source = "(+ (* 2 42) (/ 133 10) (- 1 6))";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 84 + 13 - 5);
	}

	std::string boolStr(bool b) {
		return b ? "true" : "false";
	}
	
	void testNot(Interpreter &interpreter)
	{
		bool values[] = { false, true };
		for (bool value : values) {
			Source source = "(! " + boolStr(value) + ")";
			Value result = execute(interpreter, source);
			assertEquals(result.type(), Value::TBoolean);
			assertEquals(result.boolean(), !value);
		}
	}

	void testOr(Interpreter &interpreter)
	{
		bool values[] = { false, true };
		for (bool a : values) {
			for (bool b : values) {
				Source source = "(|| " + boolStr(a) + " " + boolStr(b) + ")";
				Value result = execute(interpreter, source);
				assertEquals(result.type(), Value::TBoolean);
				assertEquals(result.boolean(), a || b);
			}
		}
	}

	void testAnd(Interpreter &interpreter)
	{
		bool values[] = { false, true };
		for (bool a : values) {
			for (bool b : values) {
				Source source = "(&& " + boolStr(a) + " " + boolStr(b) + ")";
				Value result = execute(interpreter, source);
				assertEquals(result.type(), Value::TBoolean);
				assertEquals(result.boolean(), a && b);
			}
		}
	}

	void testModByZero(Interpreter &interpreter)
	{
		Source source = "(% 42 0)";
		try
		{
			execute(interpreter, source);
		}
		catch (ExternalFunctionError e)
		{
			assertEquals("cannot mod by zero in external function '%'", e.what());
		}
	}

	void testDivisionByZero(Interpreter &interpreter)
	{
		Source source = "(/ 42 0)";
		try
		{
			execute(interpreter, source);
		}
		catch (ExternalFunctionError e)
		{
			assertEquals("cannot divide by zero in external function '/'", e.what());
		}
	}

	void testVariablesInGlobalScope(Interpreter &interpreter)
	{
		Source source;
		source << "(var global 1)";
		source << "(set global (+ global 1))";
		source << "global";
		Value result = execute(interpreter, source);
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
		Value result = execute(interpreter, source);
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
		Value result = execute(interpreter, source);
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
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

	void testClosureSeesUpdatedVariableInOuterScope(Interpreter &interpreter)
	{
		Source source;
		source << "(defun outer ()";
		source << "  (var capture 42)";
		source << "  (defun inner () capture)";
		source << "  (set capture 13)";
		source << "  (inner))";
		source << "(outer)";
		Value result = execute(interpreter, source);
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
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 2);
	}

	void testReturnedClosureCanStillAccessVariableInOuterScope(Interpreter &interpreter)
	{
		Source source;
		source << "(defun outer ()";
		source << "  (var capture 13)";
		source << "  (defun inner () capture)";
		source << "  inner)";
		source << "(var closure (outer))";
		source << "(closure)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 13);
	}

	void testClosure(Interpreter &interpreter)
	{
		Identifier x = Identifier("x");
		Identifier y = Identifier("y");

		InstructionList inner;
		{
			inner.push_back(Instruction::refClosure(CURRENT_SOURCE_LOCATION, x));
			inner.push_back(Instruction::refClosure(CURRENT_SOURCE_LOCATION, y));
			const Value *value = interpreter.global(Identifier("+"));
			assertTrue(value, "Expected there is a global for '+'");
			inner.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, *value));
			inner.push_back(Instruction::call(CURRENT_SOURCE_LOCATION, 2));
		}

		InstructionList outer;
		{
			outer.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, Value::number(42)));
			outer.push_back(Instruction::initLocal(CURRENT_SOURCE_LOCATION, x));
			outer.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, Value::number(13)));
			outer.push_back(Instruction::initLocal(CURRENT_SOURCE_LOCATION, y));
			outer.push_back(Instruction::initClosure(CURRENT_SOURCE_LOCATION, x));
			outer.push_back(Instruction::initClosure(CURRENT_SOURCE_LOCATION, y));
			std::vector<Identifier> noParameters;
			InternalFunction closure(CURRENT_SOURCE_LOCATION, Identifier("inner"), noParameters, inner);
			outer.push_back(Instruction::push(CURRENT_SOURCE_LOCATION, Value::function(closure)));
			outer.push_back(Instruction::close(CURRENT_SOURCE_LOCATION, 2));
			outer.push_back(Instruction::call(CURRENT_SOURCE_LOCATION, 0));
		}

		Value result = interpreter.exec(outer);
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
		Value result = execute(interpreter, source);
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
		Value result = execute(interpreter, source);
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
		Value result = execute(interpreter, source);
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
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 45);
	}

	void testVariableDeclarationWithNumberType(Interpreter &interpreter)
	{
		Source source;
		source << "(var result:number 13)";
		source << "result";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 13);
	}

	void testVariableDeclarationWithStringType(Interpreter &interpreter)
	{
		Source source;
		source << "(var result:string \"Hello, World\")";
		source << "result";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TString);
		assertEquals(result.string(), "Hello, World");
	}

	void testVariableDeclarationWithBooleanType(Interpreter &interpreter)
	{
		Source source;
		source << "(var result:boolean true)";
		source << "result";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TBoolean);
		assertEquals(result.boolean(), true);
	}

	void testFunctionDeclarationsWithTypes(Interpreter &interpreter)
	{
		Source source;
		source << "(defun double:number (x:number) (* x 2))";
		source << "(var result (double 42))";
		source << "result";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 84);
	}

	void testTypeDeclarationWithMemberTypes(Interpreter &interpreter)
	{
		Source source;
		source << "(type Person id:number name:string)";
		source << "(var alice (new Person 42 \"Alice\"))";
		source << "alice.name";
		Value result = execute(interpreter, source);
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
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 55);
	}

	void testCallingDeeplyNestedFunctions(Interpreter &interpreter)
	{
		Source source;
		source << "(defun test1 () 1)";
		source << "(defun test2 () (+ 2 (test1)))";
		source << "(defun test3 () (+ 3 (test2)))";
		source << "(defun test4 () (+ 4 (test3)))";
		source << "(defun test5 () (+ 5 (test4)))";
		source << "(test5)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 15);
	}

	void testImmediateFunctionCall(Interpreter &interpreter)
	{
		Source source = "((defun immediate_function_call () 42))";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

	void testFunctionArguments(Interpreter &interpreter)
	{
		Source source = "(defun f (x y z) (concat \"x: \" x \", y: \" y \", z: \" z))";
		source << "(f 1 2 3)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TString);
		assertEquals(result.string(), "x: 1, y: 2, z: 3");
	}

	void testConditionalTrue(Interpreter &interpreter)
	{
		Source source = "(if true 42)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

	void testConditionalNonZeroNumber(Interpreter &interpreter)
	{
		Source source = "(if 1 42)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

	void testConditionalNonEmptyString(Interpreter &interpreter)
	{
		Source source = "(if \"Yep\" 42)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

	void testConditionalFalse(Interpreter &interpreter)
	{
		Source source = "(if false 42)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNil);
	}

	void testConditionalZero(Interpreter &interpreter)
	{
		Source source = "(if 0 42)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNil);
	}

	void testConditionalEmptyString(Interpreter &interpreter)
	{
		Source source = "(if \"\" 42)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNil);
	}

	void testConditionalWithElseWhenTrue(Interpreter &interpreter)
	{
		Source source = "(if true 13 else 42)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 13);
	}

	void testConditionalWithElseWhenFalse(Interpreter &interpreter)
	{
		Source source = "(if false 13 else 42)";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

	void testElseNotAllowedStartOfList(Interpreter &interpreter)
	{
		Source source = "(else true 42)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Keyword 'else' cannot be used at the start of a list");
		}
	}

	void testElseNotAllowedEndOfList(Interpreter &interpreter)
	{
		Source source = "(if true else)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Keyword 'else' cannot be used at the end of a list");
		}
	}

	void testDuplicateElseIsNotAllowed(Interpreter &interpreter)
	{
		Source source = "(if true 1 else 2 else 3)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Keyword 'else' cannot be used inside an existing 'else' block");
		}
	}

	void testDuplicateIfIsNotAllowed(Interpreter &interpreter)
	{
		Source source = "(if true 1 else 2 if 3)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Keyword 'if' must be first element of a list");
		}
	}

	void testComments(Interpreter &interpreter)
	{
		Source source;
		source << "// Single line comment";
		source << "// (println \"this is commented and should not be printed\")";
		source << "/* block comments */ (+ 13 /* but this is */ 42)";
		source << "/* (println \"this should not be printed\") */";
		source << "/*";
		source << "    Multi-line";
		source << "    Block comment";
		source << "*/";
		source << "/* Interesting */ /**/ /* comment */";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 55);
	}


	void testStringConcatenation(Interpreter &interpreter)
	{
		Source source = "(concat \"number: \" 42 \", boolean: \" true)";
		Value result = execute(interpreter, source);
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
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Identifier 'undefinedVariable' not defined");
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
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Unknown type 'UndefinedType'");
		}
	}

	void testDefunOnItsOwn(Interpreter &interpreter)
	{
		Source source = "(defun)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Keyword 'defun' function requires a name");
		}
	}

	void testDefunWithInvalidFunctionName(Interpreter &interpreter)
	{
		Source source = "(defun 42 (x) x)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Declaration expected, but got Number");
		}
	}

	void testDefunWithFunctionNameButNoArgumentsOrBody(Interpreter &interpreter)
	{
		Source source = "(defun foo)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Keyword 'defun' function requires parameter list");
		}
	}

	void testDefunWithFunctionNameAndArgumentsButWithoutBody(Interpreter &interpreter)
	{
		Source source = "(defun foo (x y))";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Keyword 'defun' function lacks a body");
		}
	}

	void testDefunWithNonListArguments(Interpreter &interpreter)
	{
		Source source = "(defun foo x 42)";
		Token token = lex(source);
		Declarations declarations = interpreter.declarations();
		try
		{
			parse(token, declarations, interpreter.settings());
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Keyword 'defun' function parameter list is incorrect");
		}
	}

	void testCallingNonFunctionalValue(Interpreter &interpreter)
	{
		Source source = "(42)";
		try
		{
			execute(interpreter, source);
			fail("Expected ExecutionError");
		}
		catch (const ExecutionError &e)
		{
			assertEquals(e.what(), "Call instruction expects top of the stack to be functional value, but got: 42");
		}
	}

	std::string MATHS[] = { "+", "-", "*", "/", "%", ">", "<", ">=", "<=" };

	bool acceptsMoreThanTwoArguments(const std::string &maths)
	{
		return maths == "+" || maths == "*";
	}

	void testMathWithNoArguments(Interpreter &interpreter)
	{
		for (std::string math : MATHS)
		{
			Source source = "(" + math + ")";
			try
			{
				execute(interpreter, source);
				fail("Expected ExecutionError");
			}
			catch (const ExecutionError &e)
			{
				if (acceptsMoreThanTwoArguments(math))
				{
					assertEquals(e.what(), "Expected at least 2 arguments in external function '" + math + "'");
				}
				else
				{
					assertEquals(e.what(), "Expected 2 arguments in external function '" + math + "'");
				}
			}
		}
	}

	void testMathWithOneArgument(Interpreter &interpreter)
	{
		for (std::string math : MATHS)
		{
			Source source = "(" + math + " 42)";
			try
			{
				execute(interpreter, source);
				fail("Expected ExecutionError");
			}
			catch (const ExecutionError &e)
			{
				if (acceptsMoreThanTwoArguments(math))
				{
					assertEquals(e.what(), "Expected at least 2 arguments in external function '" + math + "'");
				}
				else
				{
					assertEquals(e.what(), "Expected 2 arguments in external function '" + math + "'");
				}
			}
		}
	}

	void testMathWithNonNumericArguments(Interpreter &interpreter)
	{
		for (std::string math : MATHS)
		{
			Source source = "(" + math + " \"forty\" 2)";
			try
			{
				execute(interpreter, source);
				fail("Expected ExecutionError");
			}
			catch (const ExecutionError &e)
			{
				assertEquals(e.what(), "Expected numeric argument in external function '" + math + "'");
			}
		}
	}

	void testComparingFunctionsIsNotSupported(Interpreter &interpreter)
	{
		Source source;
		source << "(defun foo () 42)";
		source << "(== foo foo)";
		try
		{
			execute(interpreter, source);
			fail("Expected ExecutionError");
		}
		catch (const ExecutionError &e)
		{
			assertEquals(e.what(), "Comparing functions is not supported");
		}
	}

	void testComparingTypesIsNotSupported(Interpreter &interpreter)
	{
		Source source;
		source << "(type Person id name)";
		source << "(== Person Person)";
		try
		{
			execute(interpreter, source);
			fail("Expected ExecutionError");
		}
		catch (const ExecutionError &e)
		{
			assertEquals(e.what(), "Comparing types is not supported");
		}
	}

	void testGlobalTypeIsAvailableInFunction(Interpreter &interpreter)
	{
		Source source;
		source << "(type Person id name)";
		source << "(defun globalType ()";
		source << "  (new Person 42 \"Alice\"))";
		source << "(var person (globalType))";
		source << "person.id";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

	void testLocalTypeIsAvailableInsideDefiningFunction(Interpreter &interpreter)
	{
		Source source;
		source << "(defun localType ()";
		source << "  (type Person id name)";
		source << "  (new Person 42 \"Alice\"))";
		source << "(var person (localType))";
		source << "person.id";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

	void testLocalTypeIsNotAvailableOutsideDefiningFunction(Interpreter &interpreter)
	{
		Source source;
		source << "(defun inaccessibleType ()";
		source << "  (type Person id name))";
		source << "(var person (new Person 42 \"Alice\"))";
		source << "person.id";
		try
		{
			execute(interpreter, source);
			fail("Expected ParseError");
		}
		catch (const ParseError &e)
		{
			assertEquals(e.what(), "Identifier 'Person' not defined");
		}
	}

	void testTypeDefinitionWithPrimitiveTypedMembers(Interpreter &interpreter)
	{
		Source source;
		source << "(type Person id:number name:string)";
		source << "(var person (new Person 42 \"Alice\"))";
		source << "person.id";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}

#if 0
	// TODO: fix custom types as type limiters
	void testTypeDefinitionWithCustomTypedMembers(Interpreter &interpreter)
	{
		Source source;
		source << "(type Node parent:Node value:number)";
		source << "(var node (new Node nil 42))";
		source << "node.id";
		Value result = execute(interpreter, source);
		assertEquals(result.type(), Value::TNumber);
		assertEquals(result.number(), 42);
	}
#endif
}

namespace
{
	typedef void InterpreterTest(Interpreter &);

	struct UnitTest {
		const char *name;
		InterpreterTest *function;
	};

	int runUnitTest(UnitTest unitTest, const Settings &settings)
	{
		try
		{
			std::cout << "Running " << unitTest.name << " ..." << '\n';
			Interpreter::Globals globals;
			standardMath(globals);
			standardLibrary(globals);
			Interpreter interpreter(globals, settings);
			unitTest.function(interpreter);
			std::cout << "PASSED " << flushAssertions() << " assertions\n";
			return 0;
		}
		catch(const RaspError &e)
		{
			std::cerr << "ERROR: " << e.what() << '\n';
			printStackTrace(std::cerr, e);
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

#define TEST_CASE(testName) UnitTest { #testName, &testName }

static UnitTest tests[] = {
	TEST_CASE(testParser),
	TEST_CASE(testInterpreter),
	TEST_CASE(testMathExpression),
	TEST_CASE(testNot),
	TEST_CASE(testOr),
	TEST_CASE(testAnd),
	TEST_CASE(testModByZero),
	TEST_CASE(testDivisionByZero),
	TEST_CASE(testVariablesInGlobalScope),
	TEST_CASE(testGlobalsReferencesInFunction),
	TEST_CASE(testLocalsInFunction),
	TEST_CASE(testClosureCanAccessVariableInOuterScope),
	TEST_CASE(testClosureSeesUpdatedVariableInOuterScope),
	TEST_CASE(testClosureCanModifyVariableInOuterScope),
	TEST_CASE(testReturnedClosureCanStillAccessVariableInOuterScope),
	TEST_CASE(testClosure),
	TEST_CASE(testTypesAndMemberAccess),
	TEST_CASE(testSimpleLoop),
	TEST_CASE(testComplexLoop),
	TEST_CASE(testLoopWithInnerVariableDeclaration),
	TEST_CASE(testVariableDeclarationWithNumberType),
	TEST_CASE(testVariableDeclarationWithStringType),
	TEST_CASE(testVariableDeclarationWithBooleanType),
	TEST_CASE(testFunctionDeclarationsWithTypes),
	TEST_CASE(testTypeDeclarationWithMemberTypes),
	TEST_CASE(testFunctionRecursion),
	TEST_CASE(testCallingDeeplyNestedFunctions),
	TEST_CASE(testImmediateFunctionCall),
	TEST_CASE(testFunctionArguments),
	TEST_CASE(testConditionalTrue),
	TEST_CASE(testConditionalNonZeroNumber),
	TEST_CASE(testConditionalNonEmptyString),
	TEST_CASE(testConditionalFalse),
	TEST_CASE(testConditionalZero),
	TEST_CASE(testConditionalEmptyString),
	TEST_CASE(testConditionalWithElseWhenTrue),
	TEST_CASE(testConditionalWithElseWhenFalse),
	TEST_CASE(testElseNotAllowedStartOfList),
	TEST_CASE(testElseNotAllowedEndOfList),
	TEST_CASE(testDuplicateElseIsNotAllowed),
	TEST_CASE(testDuplicateIfIsNotAllowed),
	TEST_CASE(testComments),
	TEST_CASE(testStringConcatenation),
	TEST_CASE(testReferenceToUndefinedVariable),
	TEST_CASE(testVariableDeclarationWithUndefinedType),
	TEST_CASE(testDefunOnItsOwn),
	TEST_CASE(testDefunWithInvalidFunctionName),
	TEST_CASE(testDefunWithFunctionNameButNoArgumentsOrBody),
	TEST_CASE(testDefunWithFunctionNameAndArgumentsButWithoutBody),
	TEST_CASE(testDefunWithNonListArguments),
	TEST_CASE(testCallingNonFunctionalValue),
	TEST_CASE(testMathWithNoArguments),
	TEST_CASE(testMathWithOneArgument),
	TEST_CASE(testMathWithNonNumericArguments),
	TEST_CASE(testComparingFunctionsIsNotSupported),
	TEST_CASE(testComparingTypesIsNotSupported),
	TEST_CASE(testGlobalTypeIsAvailableInFunction),
	TEST_CASE(testLocalTypeIsAvailableInsideDefiningFunction),
	TEST_CASE(testLocalTypeIsNotAvailableOutsideDefiningFunction),
	TEST_CASE(testTypeDefinitionWithPrimitiveTypedMembers),
	// TEST_CASE(testTypeDefinitionWithCustomTypedMembers),
};

int runUnitTests(const Settings &settings)
{
	int result = 0;
	for (UnitTest test: tests)
	{
		result += runUnitTest(test, settings);
	}
	return result;
}

