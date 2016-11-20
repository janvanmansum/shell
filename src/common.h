/*
 * Verwijder commentaartekens om het project te bouwen met debug-log geactiveerd. De DEBUG-macros voegen in
 * dat geval statements toe die berichten naar de standaard error schrijven en die ook direct flushen. De handigste
 * manier om deze logs te scheiden van de meeste reguliere uitvoer is door de shell als volgt op te starten:
 *
 * ./shell 2> debug.log
 *
 * en vervolgens in een apart terminal-venster:
 *
 * tail -f debug.log
 *
 * Dit werkt in ieder geval in bash en zsh.
 */
//#define LOG

#ifndef COMMON_H
#define COMMON_H

// Maximaal aantal commando's dat in één keer kan worden gestart. Hier wordt geen controle op uitgevoerd, dus gedrag
// bij overschrijding is ongedefinieerd.
#define MAXCOMMANDS    20

// Maximaal aantal chars voor één commando-regel. Hier wordt ook niet op gecontroleerd.
#define MAXLINE 	 4096

// Maximum aantal argumenten per commando. Ook weer geen controle.
#define MAXARGS		   20

#include <stdbool.h>

/*
 * Beschrijving van een uit te voeren commando
 */
typedef struct {
	// Commando + argumenten
	char *args[MAXARGS + 1];

	// Naam van een bestand om de standaard invoer uit te lezen, of NULL
	char *input;

	// Naam van een bestand om de standaard uitvoer naartoe te schrijven, of NULL
	char *output;

	// Volgnummer van de pipe om de standaard invoer uit te lezen, of -1
	int in_pipe_index;

	// Volgnummer van de pipe om de standaard uitvoer naartoe te schrijven, of -1
	int out_pipe_index;
} COMMAND;

#endif
