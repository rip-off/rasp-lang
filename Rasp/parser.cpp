#include "parser.h"

#include <cctype>
#include <algorithm>

#include "utils.h"
#include "exceptions.h"

typedef std::string::const_iterator Iterator;

namespace
{
	class MatchParens
	{
	public:
		MatchParens() 
			: diff(1) 
		{
		}

		bool operator()( char c )
		{
			if(c == '(')
			{
				++diff;
			}
			else if(c == ')')
			{
				--diff;
				if(diff == 0)
				{
					return true;
				}
			}
			return false;
		}
	private:
		unsigned diff;
	};

	class IsSpace
	{
	public:
		IsSpace(bool negate = false) : negate(negate)
		{
		}

		bool operator()(char c)
		{
			bool space = std::isspace(c);
			return negate != space; // negate ? !space : space;
		}
	private:
		bool negate;
	};

	Iterator consumeWhitespace(const Iterator begin, const Iterator end)
	{
		return std::find_if(begin, end, IsSpace(true));
	}

	Token literal(Iterator &current, const Iterator end)
	{
		current = consumeWhitespace(current, end);
		const Iterator literalEnd = std::find_if(current, end, IsSpace());
		
		std::string string(current, literalEnd);
		current = literalEnd;
		
		if(string == "nil")
		{
			return Token::nil();
		}
		else if(is<int>(string))
		{
			return Token::number(string);
		}
		else
		{
			return Token::identifier(string);
		}
	}

	Token next(Iterator &begin, const Iterator end);

	Token list(Iterator &current, const Iterator end)
	{
		current = consumeWhitespace(current, end);
		const Iterator endOfList = std::find_if(current, end, MatchParens());
		if(endOfList == end)
		{
			throw ParseError("Unterminated list");
		}

		Token result = Token::list();
		while(current != endOfList)
		{
			result.addChild(next(current, endOfList));
		}

		// This should be 
		if(current != end)
		{
			++current;
		}
		return result;
	}

	Token next(Iterator &current, const Iterator end)
	{
		current = consumeWhitespace(current, end);
		if(current == end)
		{
			return Token();
		}
		else
		{
			char c = *current;
			if(c == ')')
			{
				throw ParseError("Stray ) in program");
			}
			else if(c == '(')
			{
				++current;
				return list(current, end);
			}
			else
			{
				return literal(current, end);
			}
		}
	}
} // End anonymous namespace

Token parse(const std::string &source)
{
	Token root;
	
	Iterator it = source.begin();
	const Iterator end = source.end();
	while(it != end)
	{
		Token token = next(it, end);
		root.addChild(token);
	}

	return root;
}
