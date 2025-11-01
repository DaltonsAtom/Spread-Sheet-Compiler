#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "ir.h"

/**
 * @brief Optimizes the given bytecode array in place.
 *
 * @param code The CodeArray to optimize.
 */
void optimize_bytecode(CodeArray* code);

#endif // OPTIMIZER_H

