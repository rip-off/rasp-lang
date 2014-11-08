#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <string>

class Identifier
{
public:
	explicit Identifier(const std::string &name) : name_(name)
	{
	}

	const std::string /*&*/name() const
	{
		return name_;
	}

private:
	std::string name_;
};

inline bool operator==(const Identifier &a, const Identifier &b) {
	return a.name() == b.name();
}

inline bool operator<(const Identifier &a, const Identifier &b) {
	return a.name() < b.name();
}

#endif

