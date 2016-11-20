#ifndef RUN_H
#define RUN_H
/*
 * Module die verantwoordelijk is voor het uitvoeren van commando's.
 */

#include "common.h"

/*
 * Start alle commando's in de array "commands" op met pipes tussen elk commando en het volgende.
 * Als "blocking" true is, wacht de functie tot alle commando's getermineerd zijn voordat zij terugkeert.
 * Als "blocking" false is, keert de functie direct na het opstarten terug.
 */
void run_commands(COMMAND (*commands)[], int ncmd, bool blocking);

/*
 * Sluit het programma beleefd af.
 */
void exit_shell(void);


#endif
