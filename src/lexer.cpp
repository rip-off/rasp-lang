#include "lexer.h"

#include <cctype>
#include <iterator>
#include <algorithm>

#include "bug.h"
#include "utils.h"
#include "escape.h"
#include "keyword.h"
#include "exceptions.h"

namespace
{
	class Iterator : public std::iterator<std::forward_iterator_tag, char>
	{
	public:
		Iterator(const std::string &filename, std::string::const_iterator it)
		:
			line_(1),
			filename_(filename),
			it(it)
		{
		}

		char operator*() const
		{
			return *it;
		}

		SourceLocation sourceLocation() const
		{
			return SourceLocation(filename_, line_);
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

		Iterator next() const
		{
			Iterator copy = *this;
			++copy;
			return copy;
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
		std::string filename_;
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
			return negate != space;
		}
	private:
		bool negate;
	};

	Iterator consumeWhitespace(const Iterator begin, const Iterator end)
	{
		return std::find_if(begin, end, IsSpace(true));
	}

	Iterator consumeComment(const Iterator begin, const Iterator end)
	{
		if (begin == end)
		{
			return end;
		}

		char c = *begin;
		if(c != '/')
		{
			return begin;
		}

		Iterator next = begin.next();
		if (next == end)
		{
			// Handles (/), i.e. division with no arguments
			return begin;
		}

		if(*next == '/')
		{
			// Single line comment, consume to end of line
			return std::find(next, end, '\n');
		}
		
		if(*next == '*')
		{
			Iterator current = next;
			// Block line comment, consume until end
			SourceLocation start = next.sourceLocation();
			bool found = false;
			while(!found)
			{
				current = std::find(current, end, '*');
				if(current == end)
				{
					throw LexError(start, "Cannot find end of block comment");
				}
				++current; // Skip asterisk
				if(*current == '/')
				{
					++current; // Skip close comment
					found = true;
				}
			}
			return current;
		}
		return begin;
	}

	void consumeCommentsAndWhitespace(Iterator &current, const Iterator end)
	{
		// Keep looping to handle cases like
		// /* */ /* */ ...
		bool loop;
		do
		{
			Iterator begin = current;
			current = consumeWhitespace(current, end);
			current = consumeComment(current, end);
			loop = (begin != current);
		}
		while(loop);
	}

	Identifier tryMakeIdentifier(const SourceLocation &sourceLocation, const std::string &string)
	{
		if (!Identifier::isValid(string))
		{
			throw LexError(sourceLocation, "Illegal identifier '" + string + "'");
		}
		return Identifier(string);
	}

	Token declarationOrIdentifierOrMemberAccess(const SourceLocation &sourceLocation, const std::string string)
	{
		const std::string::const_iterator end = string.end();
		std::string::const_iterator current = string.begin();

		std::string::const_iterator colonDelimiter = std::find(current, end, ':');
		if (colonDelimiter != end)
		{
			std::string name = std::string(current, colonDelimiter);
			std::string type = std::string(colonDelimiter + 1, end);
			Declaration declaration = Declaration(tryMakeIdentifier(sourceLocation, name), type);
			return Token::declaration(sourceLocation, declaration);
		}

		std::string::const_iterator dotDelimiter = std::find(current, end, '.');
		std::string name = std::string(current, dotDelimiter);
		Token identifier = Token::identifier(sourceLocation, tryMakeIdentifier(sourceLocation, name));

		while(dotDelimiter != end)
		{
			current = dotDelimiter + 1;
			dotDelimiter = std::find(current, end, '.');

			std::string memberName = std::string(current, dotDelimiter);
			Token memberAccess = Token::identifier(sourceLocation, tryMakeIdentifier(sourceLocation, memberName));
			identifier.addChild(memberAccess);
		}

		return identifier;
	}

	Token literal(Iterator &current, const Iterator end)
	{
		current = consumeWhitespace(current, end);
		const Iterator literalEnd = std::find_if(current, end, IsSpace());
		
		std::string string(current, literalEnd);
		current = literalEnd;

		if(isKeyword(string))
		{
			return Token::keyword(current.sourceLocation(), string);
		}
		else if(is<int>(string))
		{
			return Token::number(current.sourceLocation(), string);
		}
		else
		{
			return declarationOrIdentifierOrMemberAccess(current.sourceLocation(), string);
		}
	}

	Token next(Iterator &begin, const Iterator end);

	Token list(Iterator &current, const Iterator end)
	{
		current = consumeWhitespace(current, end);
		const Iterator endOfList = std::find_if(current, end, MatchParens());
		if(endOfList == end)
		{
			throw LexError(current.sourceLocation(), "Unterminated list");
		}

		Token result = Token::list(current.sourceLocation());
		while(current != endOfList)
		{
			Token token = next(current, endOfList);
			result.addChild(token);
			consumeCommentsAndWhitespace(current, endOfList);
		}

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
					throw LexError(current.sourceLocation(), message);
				}
				escape = false;
			}
			else
			{
				if (c == '\"')
				{
					++current;
					return Token::string(current.sourceLocation(), text);
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
		throw LexError(current.sourceLocation(), "String literal never closed");
	}

	Token next(Iterator &current, const Iterator end)
	{
		consumeCommentsAndWhitespace(current, end);
		if(current == end)
		{
			throw CompilerBug("Cannot extract a token from the end of the source: " + str(current.sourceLocation()));
		}
		else
		{
			char c = *current;
			if(c == ')')
			{
				throw LexError(current.sourceLocation(), "Stray ) in program");
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

Token lex(const std::string &filename, const std::string &source)
{
	Token root = Token::list(SourceLocation(filename, 0));
	
	Iterator it = Iterator(filename, source.begin());
	const Iterator end = Iterator(filename, source.end());
	while(it != end)
	{
		Token token = next(it, end);
		root.addChild(token);
		consumeCommentsAndWhitespace(it, end);
	}

	return root;
}

