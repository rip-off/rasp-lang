#ifndef ESCAPE_H
#define ESCAPE_H

bool needsEscaping(char c);
bool needsReescaping(char c);

char unescape(char c);
char reescape(char c);

#endif

