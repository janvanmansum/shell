#ifndef PARSER_H
#define PARSER_H
/*
 * Module voor het parseren van de invoerregel. Terugkoppeling van het resultaat gebeurt door middel van
 * de COMMAND structure, gedefinieerd in common.h.
 *
 * LET OP: een aantal functies in deze module verandert de invoerregel.
 */

#include "common.h"

/*
 * Analyseert de commandoregel in "line" en zet deze om in een reeks van COMMAND-structures, waarvan elk een van
 * een met pipes verbonden keten van programma-aanroepen beschrijft. Retourneert de lengte van de gevonden keten.
 *
 * Deze functie gebruikt de standaardbibliotheekfunctie strtok, die de invoerregel verandert. Na aanroep is elk teken
 * na een commando, argument, redirect vervangen door een char met waarde 0. De invoer is dus niet herbruikbaar.
 */
int get_commands(char line[], COMMAND (*commands)[]);

/*
 * Retourneert of "line" enkel uit witruimte bestaat.
 */
bool is_blank(char line[]);

/*
 * Bepaalt of de commandoregel een blocking call voorstelt (zonder &) of een background call.
 * Als de & gevonden wordt, wordt deze ook vervangen door een char met waarde 0.
 */
bool is_blocking_call(char line[]);

#endif
