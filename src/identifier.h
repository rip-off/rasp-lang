#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <string>

class Identifier
{
public:
	explicit Identifier(const std::string &name);

	const std::string &name() const
	{
		return name_;
	}

	static bool isValid(const std::string &name);

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

