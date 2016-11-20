/*
 * Als de LOG-macro gedefinieerd is voegen de volgende macro's statements toe die naar de standaard error loggen.
 * Anders worden de macro's naar een lege string geëxpandeerd.
 */

#ifdef LOG
#define DEBUG(msg, arg)	\
	fprintf(stderr, "[%d] ", (int) getpid()); \
	fprintf(stderr, msg, arg); \
	fprintf(stderr, "\n"); fflush(stderr);
#define DEBUG2(msg, arg1, arg2)	\
		fprintf(stderr, "[%d] ", (int) getpid()); \
		fprintf(stderr, msg, arg1, arg2); \
		fprintf(stderr, "\n"); fflush(stderr);
#endif

#ifndef LOG
#define DEBUG(msg, arg)
#define DEBUG2(msg, arg1, arg2)
#endif

