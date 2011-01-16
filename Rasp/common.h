#ifndef COMMON_H
#define COMMON_H

#include <vector>

#include "value.h"

typedef std::vector<Value> Arguments;
typedef Value ApiFunction(const Arguments &args);

#endif
