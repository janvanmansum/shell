#include "common.h"
#include "terminal.h"
#include <stdio.h>

void prompt(void) {
	printf("$ ");
}

bool get_next_line(char line[]) {
	if (fgets(line, MAXLINE, stdin) != NULL) {
		line[strlen(line) - 1] = 0;
		return true;
	}
	return false;
}
