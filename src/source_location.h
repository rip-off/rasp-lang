#ifndef SOURCE_LOCATION_H
#define SOURCE_LOCATION_H

#include <string>
#include <iosfwd>

class SourceLocation
{
public:
	SourceLocation(const std::string &filename, unsigned line);

	unsigned line() const;

    friend std::ostream &operator<<(std::ostream &out, const SourceLocation &);
private:
	unsigned line_;
	std::string filename_;
};

#define CURRENT_SOURCE_LOCATION SourceLocation(__FILE__, __LINE__)

#endif

