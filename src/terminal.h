#ifndef TERMINAL_H
#define TERMINAL_H

/*
 * Module met hulpfuncties voor de communicatie met de terminal-gebruiker.
 */

#include <stdbool.h>

/*
 * Drukt een prompt af op het scherm.
 */
void prompt(void);

/*
 * Leest de volgende regel die de gebruiker ingeeft en plaatst die in de buffer "line".
 */
bool get_next_line(char line[]);

#endif
