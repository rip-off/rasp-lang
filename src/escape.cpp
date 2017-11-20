#include "escape.h"

#include <stdexcept>

namespace {
	struct Escape {
		char source; // e.g. n for newline
		char replacement; // e.g. the newline character for \n
	};

	const Escape escape[] = {
		{'\\', '\\'},
		{'\"', '\"'},
		{'n' , '\n'},
	};

	char throwUnrecognisedEscape(char c)
	{
		std::string message = "Not an escape character: " ;
		message += c;
		throw std::logic_error(message);
	}
}

bool needsEscaping(char c)
{
	for (int i = 0 ; i < sizeof(escape) / sizeof(escape[0]) ; ++i)
	{
		if (c == escape[i].source)
		{
			return true;
		}
	}
	return false;
}

bool needsReescaping(char c)
{
	for (int i = 0 ; i < sizeof(escape) / sizeof(escape[0]) ; ++i)
	{
		if (c == escape[i].replacement)
		{
			return true;
		}
	}
	return false;
}


char unescape(char c)
{
	for (int i = 0 ; i < sizeof(escape) / sizeof(escape[0]) ; ++i)
	{
		if (c == escape[i].source)
		{
			return escape[i].replacement;
		}
	}
	return throwUnrecognisedEscape(c);
}

char reescape(char c)
{
	for (int i = 0 ; i < sizeof(escape) / sizeof(escape[0]) ; ++i)
	{
		if (c == escape[i].replacement)
		{
			return escape[i].source;
		}
	}
	return throwUnrecognisedEscape(c);
}

std::string addEscapes(const std::string &text)
{
	std::string result;
	for (unsigned i = 0 ; i < text.size() ; ++i)
	{
		char c = text[i];
		if (needsReescaping(c))
		{
			result += '\\';
			result += reescape(c);
		}
		else
		{
			result += c;
		}
	}
	return result;
}

