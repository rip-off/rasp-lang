#include "keyword.h"

#include "utils.h"

const std::string KEYWORD_IF = "if";
const std::string KEYWORD_VAR = "var";
const std::string KEYWORD_SET = "set";
const std::string KEYWORD_ELSE = "else";
const std::string KEYWORD_TYPE = "type";
const std::string KEYWORD_DEFUN = "defun";
const std::string KEYWORD_WHILE = "while";

namespace
{
	const std::string KEYWORDS[] =
	{
		KEYWORD_IF,
		KEYWORD_VAR,
		KEYWORD_SET,
		KEYWORD_ELSE,
		KEYWORD_TYPE,
		KEYWORD_DEFUN,
		KEYWORD_WHILE
	};
}

bool isKeyword(const std::string &string)
{
	return array_is_element(KEYWORDS, string);
}

