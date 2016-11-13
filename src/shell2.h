//#define TEST2
#define PROD
#define MAXLINE 	 4096
#define MAXCOMMANDS    20
#define MAXARGS		   20

#define INPUT_REDIRECT 	0
#define OUTPUT_REDIRECT 1
#define ARG				2

#include <stdbool.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

typedef struct {
	bool filled;
	char *args[MAXARGS + 1];
	char *input;
	char *output;
	int in_pipe_index;
	int out_pipe_index;
} COMMAND;

typedef struct {
	int npipes;
	int pipes[MAXCOMMANDS - 1][2];
} PIPES;


void create_pipes(PIPES *pipes, int npipes);

void close_pipes(PIPES *pipes);

void prompt(void);

void print_array(char *a[]);

void debug(char *msg);

/*
 * Haalt de volgende regel op. Retourneert false bij het einde van de invoer.
 */
bool get_next_line(char line[]);

bool is_blank(char line[]);

/*
 * Bepaalt of de commandoregel een blocking call voorstelt (zonder &) of een background call.
 * Als de & gevonden wordt, wordt deze ook vervangen door een null character.
 */
bool is_blocking_call(char line[]);

/*
 * Vult een array van COMMAND structures met de informatie gevonden in line. De array dient voor de aanroep
 * COMMAND structures te bevatten, waarvan het veld filled op false staat.
 */
int get_commands(char line[], COMMAND (*commands)[]);


int get_part_type(int index, char s[]);


/*
 * Vult de COMMAND structure aan de hand van de informatie gevonden in line. line mag geen pipe-tekens meer
 * bevatten; m.a.w. line mag slechts één proces starten.
 */
void get_command(char line[], COMMAND *command);

/*
 * Voert de commando's in de array commands uit, waarbij tussen de standaard uitvoer van een commando en de
 * standaard invoer van het opvolgende commando steeds een pipe wordt aangelegd. De functie wacht
 * afhankelijk van de waarde van blocking wel (1) of niet (0) op de exit status van het laatste commando.
 */
void execute_commands(COMMAND (*commands)[], int ncmd, bool blocking);



/*
 * Voer het commando cmd uit als het een ingebouwd commando is, zoals cd of exit. De returnwaarde geeft aan
 * of het commando werd herkend als built-in en uitgevoerd.
 */
bool execute_builtin(COMMAND *cmd);


void exit_shell(void);

/*
 * Voert het commando cmd uit en wacht daarbij, afhankelijk van de waarde van blocking wel of niet
 * op de exit status van het proces.
 */
int execute_command(COMMAND *cmd, bool blocking);

void exec_command(COMMAND *cmd);


/*
 * Initialiseert de velden van alle elementen in de array commands op null.
 */
void reset_commands(COMMAND (*commands)[]);

void reset_command(COMMAND *command);



int split(char s[], const char *delim, char *parts[]);




