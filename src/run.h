#ifndef RUN_H
#define RUN_H

#include "common.h"

void run_commands(COMMAND (*commands)[], int ncmd, bool blocking);

void exit_shell(void);


#endif
