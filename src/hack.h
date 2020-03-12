#ifndef HACK__GUARD_H
#define HACK__GUARD_H

/**
 * Evaluate the provided program. A return value of 0 means the program
 * executed successfully, while anything else implies an error occurred.
 */
int evaluate_program(char** lines, int length);

#endif
