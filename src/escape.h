#ifndef ESCAPE_H
#define ESCAPE_H

#include <string>

bool needsEscaping(char c);
bool needsReescaping(char c);

char unescape(char c);
char reescape(char c);

std::string addEscapes(const std::string &text);

#endif

