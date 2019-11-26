#include "keyword.h"

#include "utils.h"

const std::string KEYWORD_IF = "if";
const std::string KEYWORD_VAR = "var";
const std::string KEYWORD_SET = "set";
const std::string KEYWORD_INC = "inc";
const std::string KEYWORD_NIL = "nil";
const std::string KEYWORD_ELSE = "else";
const std::string KEYWORD_TYPE = "type";
const std::string KEYWORD_TRUE = "true";
const std::string KEYWORD_FALSE = "false";
const std::string KEYWORD_DEFUN = "defun";
const std::string KEYWORD_WHILE = "while";

namespace
{
	const std::string KEYWORDS[] =
	{
		KEYWORD_IF,
		KEYWORD_VAR,
		KEYWORD_SET,
		KEYWORD_INC,
		KEYWORD_NIL,
		KEYWORD_ELSE,
		KEYWORD_TYPE,
		KEYWORD_TRUE,
		KEYWORD_FALSE,
		KEYWORD_DEFUN,
		KEYWORD_WHILE
	};
}

bool isKeyword(const std::string &string)
{
	return array_is_element(KEYWORDS, string);
}

