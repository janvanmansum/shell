#include "common.h"
#include "terminal.h"
#include "parser.h"
#include "run.h"

#ifdef PROD
#include <stdlib.h>

int main(void) {
	char line[MAXLINE];
	COMMAND commands[MAXCOMMANDS];
	bool blocking = true;
	int ncmd = 0;

	prompt();
	while (get_next_line(line)) {
		if (is_blank(line)) {
			prompt();
			continue;
		}
		blocking = is_blocking_call(line);
		ncmd = get_commands(line, &commands);
		run_commands(&commands, ncmd, blocking);
		reset_commands(&commands);
		prompt();
	}
	exit_shell();

	return EXIT_SUCCESS;
}
#endif
