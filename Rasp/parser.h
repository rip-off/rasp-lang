#ifndef PARSER_H
#define PARSER_H

#include "bindings.h"
#include "instruction.h"

class Token;
class Settings;

InstructionList parse(const Token &tree, const Bindings &bindings, const Settings &settings);

#endif
