#ifndef COMPILER_H
#define COMPILER_H

#include "bindings.h"
#include "instruction.h"

class Token;

InstructionList compile(const Token &tree, const Bindings &bindings);

#endif
