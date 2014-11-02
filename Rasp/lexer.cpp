#include "lexer.h"

#include <cctype>
#include <iterator>
#include <algorithm>

#include "utils.h"
#include "escape.h"
#include "exceptions.h"

// TODO: typedef std::string::const_iterator Iterator;

namespace
{
	class Iterator : public std::iterator<std::forward_iterator_tag, char>
	{
	public:
		Iterator(std::string::const_iterator it)
		:
			line_(1),
			it(it)
		{
		}

		char operator*() const
		{
			return *it;
		}

		unsigned line() const
		{
			return line_;
		}

		Iterator &operator++()
		{
			if (*it == '\n')
			{
				line_ += 1;
			}
			++it;
			return *this;
		}

		bool operator==(const Iterator &other) const
		{
			return it == other.it;
		}

		bool operator!=(const Iterator &other) const
		{
			return it != other.it;
		}

	private:
		unsigned line_;
		std::string::const_iterator it;
	};


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
			return Token::nil(current.line());
		}
		else if(is<int>(string))
		{
			return Token::number(current.line(), string);
		}
		else
		{
			return Token::identifier(current.line(), string);
		}
	}

	Token next(Iterator &begin, const Iterator end);

	Token list(Iterator &current, const Iterator end)
	{
		current = consumeWhitespace(current, end);
		const Iterator endOfList = std::find_if(current, end, MatchParens());
		if(endOfList == end)
		{
			throw LexError(current.line(), "Unterminated list");
		}

		Token result = Token::list(current.line());
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

	Token stringLiteral(Iterator &current, const Iterator end)
	{
		std::string text = "";
		bool escape = false;
		for ( /* nada */ ; current != end ; ++current)
		{
			char c = *current;
			if (escape)
			{
				if (needsEscaping(c)) 
				{
					text += unescape(c);
				}
				else
				{
					std::string message = "Invalid escape sequence \'\\";
					message += c;
					message += "\' found in string literal";
					throw LexError(current.line(), message);
				}
				escape = false;
			}
			else
			{
				if (c == '\"')
				{
					++current;
					return Token::string(current.line(), text);
				}
				else if (c == '\\')
				{
					escape = true;
				}
				else
				{
					text += c;
				}
			}
		}
		// TODO: file name & line number
		throw LexError(current.line(), "String literal never closed");
	}

	Token next(Iterator &current, const Iterator end)
	{
		current = consumeWhitespace(current, end);
		if(current == end)
		{
			// TODO: ugly phantom tokens :[
			return Token::root(current.line());
		}
		else
		{
			char c = *current;
			if(c == ')')
			{
				throw LexError(current.line(), "Stray ) in program");
			}
			else if(c == '(')
			{
				++current;
				return list(current, end);
			}
			else if(c == '\"')
			{
				++current;
				return stringLiteral(current, end);
			}
			else
			{
				return literal(current, end);
			}
		}
	}
} // End anonymous namespace

Token lex(const std::string &source)
{
	Token root = Token::root(0);
	
	Iterator it = source.begin();
	const Iterator end = source.end();
	while(it != end)
	{
		Token token = next(it, end);
		root.addChild(token);
	}

	return root;
}
