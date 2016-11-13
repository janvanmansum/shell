#include "shell2.h"

#ifdef TEST2

#include <stdlib.h>
#include <string.h>


char line1[] = "ls -1 Debug";
char line2[] = "sort -r";
COMMAND c1, c2;
COMMAND cmds[MAXCOMMANDS];

int main(void) {
	get_commands("ls -1 Debug | sort -r", &cmds);

	execute_commands(&cmds, 2, false);

	char buf[4096];
	fgets(buf, 4096, stdin);
	return EXIT_SUCCESS;
}








#endif

