#ifndef PARSER_H
#define PARSER_H

#include "common.h"

int get_commands(char line[], COMMAND (*commands)[]);
void reset_commands(COMMAND (*commands)[]);
bool is_blank(char line[]);

/*
 * Bepaalt of de commandoregel een blocking call voorstelt (zonder &) of een background call.
 * Als de & gevonden wordt, wordt deze ook vervangen door een null character.
 */
bool is_blocking_call(char line[]);

#endif
