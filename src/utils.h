#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>

template<typename T, int N>
const T *array_begin(const T (&array)[N])
{
	return array + 0;
}

template<typename T, int N>
const T *array_end(const T (&array)[N])
{
	return array + N;
}

template<typename T, int N>
bool array_is_element(const T (&haystack)[N], const T &needle)
{
	for (unsigned i = 0 ; i < N ; ++i)
	{
		if (haystack[i] == needle)
		{
			return true;
		}
	}
	return false;
}

template<class T>
bool is(const std::string &arg)
{
    std::stringstream stream(arg);
    T t;
    return stream >> t && stream.eof();
}

struct bad_cast : public std::exception {};

template<class To, class From>
To to(const From &arg)
{
    std::stringstream stream;
    stream << arg;
    To result;
    if(!(stream >> result && stream.eof()))
	{
		throw bad_cast();
	}
    return result;
}

template<class Type>
std::string str(const Type &type)
{
	std::stringstream stream;
	stream << type;
	return stream.str();
}

#endif
