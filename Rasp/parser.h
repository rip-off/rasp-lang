#ifndef PARSER_H
#define PARSER_H

#include "bindings.h"
#include "instruction.h"

class Token;

InstructionList parse(const Token &tree, const Bindings &bindings);

#endif
