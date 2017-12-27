#ifndef KEYWORD_H
#define KEYWORD_H

#include <string>

extern const std::string KEYWORD_IF;
extern const std::string KEYWORD_VAR;
extern const std::string KEYWORD_SET;
extern const std::string KEYWORD_ELSE;
extern const std::string KEYWORD_TYPE;
extern const std::string KEYWORD_DEFUN;
extern const std::string KEYWORD_WHILE;

bool isKeyword(const std::string &string);

#endif

