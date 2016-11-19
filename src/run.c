#include "run.h"
#include "common.h"
#include "log.h"
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

struct {
	int size;
	int pipes[MAXCOMMANDS - 1][2];
} pipes;

static bool execute_builtin(COMMAND *cmd);
static void cd(char *dir);
static void create_pipes(int npipes);
static void install_pipes(COMMAND (*commands)[], int ncmd);
static void create_processes(COMMAND (*commands)[], bool blocking, int ncmd, pid_t (*pids)[]);
static pid_t create_process(COMMAND *cmd, bool foreground);
static void exec_command(COMMAND *cmd);
static void close_pipes();
static void wait_for_processes(COMMAND (*commands)[], int ncmd, pid_t (*pids)[]);

void run_commands(COMMAND (*commands)[], int ncmd, bool blocking) {
	DEBUG("run_commands: running %d command(s)", ncmd)
	pid_t pids[MAXCOMMANDS];

	if (execute_builtin(*commands))
		return;

	create_pipes(ncmd - 1);
	install_pipes(commands, ncmd);
	create_processes(commands, blocking, ncmd, &pids);
	close_pipes();
	wait_for_processes(commands, ncmd, &pids);
}

static bool execute_builtin(COMMAND *cmd) {
	bool command_found = false;

	if (!strcmp(cmd->args[0], "exit"))
		exit_shell();
	else if (!strcmp(cmd->args[0], "cd")) {
		cd(cmd->args[1]);
		command_found = true;
	}
	DEBUG("execute_builtin: was command built-in? %s", command_found ? "yes": "no")
	return command_found;
}

void exit_shell(void) {
	printf("Good-bye!\n");
	exit(0);
}

static void cd(char *dir) {
	char path[MAXLINE];

	if (dir) {
		if (dir[0] != '/' && dir[0] != '.') {
			strcpy(path, "./");
			strcat(path, dir);
		} else
			strcpy(path, dir);
	} else
		strcpy(path, "");

	DEBUG("cd: changing directory to '%s'", path)
	if (chdir(path) == -1) {
		fprintf(stderr, "Cannot change directory to %s\n", path);
	}
}

static void create_pipes(int npipes) {
	DEBUG("create_pipes: number of pipes = %d", npipes)
	for (int i = 0; i != npipes; ++i)
		pipe(pipes.pipes[i]);
	pipes.size = npipes;
}

static void install_pipes(COMMAND (*commands)[], int ncmd) {
	DEBUG("install_pipes: installing pipes between %d commands", ncmd)
	for (int i = 0; i < ncmd; ++i) {
		COMMAND *cmd = *commands + i;
		if (i > 0) {
			cmd->in_pipe_index = i - 1;
			COMMAND *prev_cmd = *commands + (i - 1);
			prev_cmd->out_pipe_index = i - 1;
		}
	}
}

static void create_processes(COMMAND (*commands)[], bool blocking, int ncmd, pid_t (*pids)[]) {
	DEBUG("create_processes: number of processes = %d", ncmd)
	for (int i = 0; i != ncmd	; ++i) {
		COMMAND *cmd = *commands + i;
		(*pids)[i] = create_process(cmd, blocking);
	}
}

static pid_t create_process(COMMAND *cmd, bool foreground) {
	DEBUG2("create_process: '%s', starting it in the %s", cmd->args[0], foreground ? "foreground" : "background")
	pid_t pid;
	if (foreground) {
		/*
		 * Als het proces gewoon op de voorgrond kan draaien, hoeven we maar één keer te forken en wachten we daarna
		 * gewoon op het kind-proces.
		 */
		if ((pid = fork()) < 0)
			fprintf(stderr, "Fork error");
		else if (pid == 0) {
			/*
			 * Hier zijn we in het kind-proces.
			 */
			exec_command(cmd);
		}
		DEBUG2("create_process: created process for '%s', pid = %d", cmd->args[0], pid)
	} else {
		/*
		 * Fork twee keer om zombieproces te voorkomen. Uit "Advanced Programming in the Unix Environment"
		 * 1e Editie, par 8.6, blz. 202.
		 */
		if ((pid = fork()) < 0)
			fprintf(stderr, "Fork error");
		else if (pid == 0) {
			/*
			 * Hier zijn we in het kind-proces. De enige taak van dit proces is het starten van een kleinkind-proces.
			 * Daarna exit het. Doordat het kleinkind daardoor een "wees" wordt, maakt het besturingssysteem het init-
			 * proces de ouder van het kleinkind. Het init-proces zal altijd de status van een proces ophalen, wanneer
			 * dat proces exit. Een proces dat afgesloten is, maar er nog op wacht dat zijn status wordt opgehaald,
			 * wordt een "zombie" genoemd.
			 */
			if ((pid = fork()) < 0)
				fprintf(stderr, "Fork error");

			else if (pid > 0) {
				DEBUG2("create_process: created process for '%s', pid = %d (NOT WAITING FOR COMMAND)", cmd->args[0], pid)
				exit(0);
			}

			/*
			 * Hier zijn we in het kleinkind-proces. Het enige wat we hoeven te doen is het gevraagde commando uit te
			 * voeren. Als dat niet lukt, sluiten we af met een foutcode.
			 */
			exec_command(cmd);
		}
		DEBUG2("create_process: created helper process to start '%s' in the background, pid = %d", cmd->args[0], pid)
	}

	return pid;
}

static void wait_for_processes(COMMAND (*commands)[], int ncmd, pid_t (*pids)[]) {
	DEBUG("wait_for_processes: number of commands = %d", ncmd)
	int status;
	for (int i = 0; i != ncmd; ++i) {
		DEBUG("wait_for_processes: waiting for pid %d", (*pids)[i])
		if (waitpid((*pids)[i], &status, 0) != (*pids)[i])
			fprintf(stderr, "Error waiting for command to finish: %s",
					(*commands)[i].args[0]);
	}
	DEBUG("wait_for_processes: %d command(s) returned", ncmd)
}

static void exec_command(COMMAND *cmd) {
	if (cmd->in_pipe_index != -1) {
		DEBUG("exec_command: '%s', reading STDIN from pipe", cmd->args[0])
		dup2(pipes.pipes[cmd->in_pipe_index][0], STDIN_FILENO);
	} else if (cmd->input != NULL) {
		DEBUG2("exec_command: '%s', reading STDIN from file: '%s'", cmd->args[0], cmd->input)
		int in_filedes = open(cmd->input, O_RDONLY);
		dup2(in_filedes, STDIN_FILENO);
		close(in_filedes);
	}

	if (cmd->out_pipe_index != -1) {
		DEBUG("exec_command: '%s', writing STDOUT to pipe", cmd->args[0])
		dup2(pipes.pipes[cmd->out_pipe_index][1], STDOUT_FILENO);
	} else if (cmd->output != NULL) {
		DEBUG2("exec_command: '%s', writing STDOUT to file: '%s'", cmd->args[0], cmd->output)
		int out_filedes = open(cmd->output, O_WRONLY | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		dup2(out_filedes, STDOUT_FILENO);
		close(out_filedes);
	}

	close_pipes();
	DEBUG("exec_command: '%s', calling EXEC", cmd->args[0])
	execvp(cmd->args[0], cmd->args);
	fprintf(stderr, "Could not execute command: %s\n", cmd->args[0]);
	exit(127);
}

static void close_pipes() {
	DEBUG("close_pipes: number of pipes = %d", pipes.size)
	for (int i = 0; i != pipes.size; ++i) {
		close(pipes.pipes[i][0]);
		close(pipes.pipes[i][1]);
	}
	pipes.size = 0;
}

