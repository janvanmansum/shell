//#define TEST


#define MAXLINE 	 4096
#define MAXCOMMANDS    20
#define MAXARGS		   20
#define MAXREDIRECTS   20

#include <stdbool.h>
#include <stdio.h>

typedef struct {
	bool filled;
	char *args[MAXARGS + 1];
	char *inputs[MAXREDIRECTS + 1];
	char *outputs[MAXREDIRECTS + 1];
} COMMAND;


/*
 * Haalt de volgende regel op. Retourneert false bij het einde van de invoer.
 */
bool get_next_line(char line[]);

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
void execute_commands(COMMAND (*commands)[], int blocking);

/*
 * Voer het commando cmd uit als het een ingebouwd commando is, zoals cd of exit. De returnwaarde geeft aan
 * of het commando werd herkend als built-in en uitgevoerd.
 */
bool execute_builtin(COMMAND *cmd);

/*
 * Voert het commando cmd uit en wacht daarbij, afhankelijk van de waarde van blocking wel of niet
 * op de exit status van het proces.
 */
void execute_command(COMMAND *cmd, bool blocking, FILE* in_pipe, FILE* out_pipe);


/*
 * Initialiseert de velden van alle elementen in de array commands op null.
 */
void reset_commands(COMMAND (*commands)[]);


int split(char s[], const char *delim, char *parts[]);




