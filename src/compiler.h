#ifndef COMPILER_H
#define COMPILER_H

#include <string>

class Settings;
class Interpreter;

void execute(Interpreter &, const std::string &filename, const Settings &);

#endif

