#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

/*
 * Constanten voor "part types". Deze geven aan wat voor type component er op een bepaalde plek gevonden is op
 * de invoerregel.
 */

// Een component dat aangeeft waar de standaardinvoer vandaan moet worden gelezen, bv "< inputfile.txt"
#define INPUT_REDIRECT 	0

// Een component dat aangeeft waar de standaarduitvoer naartoe moet worden geschreven, bv "> outputfile.txt"
#define OUTPUT_REDIRECT 1

// Een commando of argument voor een commando, bv "ls" of "-l"
#define ARG				2

/*
 * De volgende functies worden enkel binnen deze module aangeroepen en zijn daarom als static gedeclareerd.
 */

/*
 * Splitst de invoer "s" in delen m.b.v. de delimiters in "delim". De pointers naar de eerste tekens van de delen
 * worden in "parts" geplaatst. Elk deel wordt afgesloten door een 0-char. ("s" wordt dus veranderd.)
 */
static int split(char s[], const char *delim, char *parts[]);

/*
 * Parseert de invoer "line" om een COMMAND-structure te vullen.
 */
static void get_command(char line[], COMMAND *command);

/*
 * Bepaalt het type van een onderdeel van de commandoregel aan de hand van het teken dat er al dan niet aan voorafging.
 * Een onderdeel dat (afgezien van witruimte) voorafgegaan werd door "<" is een invoerbestand, door ">" is een uitvoerbestand
 * en elk ander onderdeel is een argument (inclusief een commandonaam).
 */
static int get_part_type(int index, char s[]) ;

/*
 * Initialiseert de velden van een COMMAND-struct.
 */
static void reset_command(COMMAND* cmd);



/*
 * Definities van publieke functies
 */


bool is_blank(char line[]) {
	for (char *p = line; *p != 0; ++p)
		if (!isspace(*p))
			return false;
	return true;
}

bool is_blocking_call(char line[]) {
	char *p;

	if ((p = strrchr(line, '&')) != NULL) {
		*p = '\0';
		return false;
	}

	return true;
}
int get_commands(char line[], COMMAND (*commands)[]) {
	char *cmdlines[MAXCOMMANDS];
	int ncmd = split(line, "|", cmdlines);
	for (int i = 0; i < ncmd; ++i) {
		get_command(cmdlines[i], (*commands) + i);
	}
	return ncmd;
}

/*
 * Definities van statische functies.
 */

static void get_command(char line[], COMMAND *command) {
	char line2[MAXLINE + 1];
	char *parts[MAXARGS + 3]; // MAXARGS + 1 input + 1 output + NULL
	int iargs = 0;
	reset_command(command);
	strcpy(line2, line);
	split(line, " \t><", parts);
	for (int i = 0; parts[i] != NULL; ++i) {
		int index = parts[i] - line;
		switch (get_part_type(index, line2)) {
		case INPUT_REDIRECT:
			command->input = parts[i];
			break;
		case OUTPUT_REDIRECT:
			command->output = parts[i];
			break;
		case ARG:
			command->args[iargs++] = parts[i];
			break;
		}
	}
	command->args[iargs] = 0;
}

static void reset_command(COMMAND* cmd) {
	cmd->args[0] = NULL;
	cmd->input = NULL;
	cmd->output = NULL;
	cmd->in_pipe_index = -1;
	cmd->out_pipe_index = -1;
}

static int split(char s[], const char *delim, char *parts[]) {
	char *p;
	int i = 0;
	p = strtok(s, delim);
	while (p != NULL) {
		if (parts != NULL)
			parts[i++] = p;
		p = strtok(NULL, delim);
	}
	if (parts != NULL)
		parts[i] = NULL;
	return i;
}

static int get_part_type(int index, char s[]) {
	for (int i = index; i != -1; --i) {
		switch (s[i]) {
		case '<':
			return INPUT_REDIRECT;
		case '>':
			return OUTPUT_REDIRECT;
		}
	}
	return ARG;
}

