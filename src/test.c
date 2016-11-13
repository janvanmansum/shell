#include "shell2.h"

#ifdef TEST

#include <stdlib.h>
#include <string.h>
static int ok = 1;

#define BCHECK(x) \
	if(x) { \
		printf("OK\t%s\n", #x); \
	} else { \
		ok = 0; \
		printf("FAIL\t%s\n", #x); \
	}; \

#define SCHECK(a, b)  \
	if(!strcmp(a, b)) { \
		printf("OK\t%s\n", #a " = " #b); \
	} else { \
		ok = 0; \
		printf("FAIL\t%s\n", #a " = " #b); \
	}; \

void test_get_part_type(void) {
	char s[] = "cmd arg1 arg2 < in1 <in2 >out1  > out2";
	BCHECK(ARG == get_part_type(0, s))
	BCHECK(ARG == get_part_type(4, s))
	BCHECK(ARG == get_part_type(9, s))
	BCHECK(INPUT_REDIRECT == get_part_type(16, s))
	BCHECK(INPUT_REDIRECT == get_part_type(21, s))
	BCHECK(OUTPUT_REDIRECT == get_part_type(26, s))
	BCHECK(OUTPUT_REDIRECT == get_part_type(34, s))
}

void test_split(void) {
	char s[] = "cmd arg1 arg2 < in1 <in2 >out1  > out2";
	char *parts[MAXARGS + 3];

	int n = split(s, " \t<>", parts);
	BCHECK(7 == n)

	SCHECK("cmd", parts[0])
	SCHECK("arg1", parts[1])
	SCHECK("arg2", parts[2])
	SCHECK("in1", parts[3])
	SCHECK("in2", parts[4])
	SCHECK("out1", parts[5])
	SCHECK("out2", parts[6])

}

void test_get_command(void) {
	COMMAND cmd;
	char s[] = "cmd arg1 arg2 < in1 <in2 >out1  > out2";

	get_command(s, &cmd);
	SCHECK("cmd", cmd.args[0])
	SCHECK("arg1", cmd.args[1])
	SCHECK("arg2", cmd.args[2])
	BCHECK(NULL == cmd.args[3])
	SCHECK("in2", cmd.input)
	SCHECK("out2", cmd.output)
}

void test_get_commands(void) {
	COMMAND cmds[MAXCOMMANDS];
	char s[] = "cmd1 arg11 | cmd2 > out21 | cmd3 arg31 arg32";

	int n = get_commands(s, &cmds);
	BCHECK(3 == n)

	// cmd1
	BCHECK(cmds[0].filled)
	SCHECK("cmd1", cmds[0].args[0])
	SCHECK("arg11", cmds[0].args[1])
	BCHECK(NULL == cmds[0].args[2])
	BCHECK(NULL == cmds[0].input)
	BCHECK(NULL == cmds[0].output)


	// cmd2
	BCHECK(cmds[1].filled)
	SCHECK("cmd2", cmds[1].args[0])
	BCHECK(NULL == cmds[1].args[1])
	BCHECK(NULL == cmds[1].input)
	SCHECK("out21", cmds[1].output)


	// cmd3
	BCHECK(cmds[2].filled)
	SCHECK("cmd3", cmds[2].args[0])
	SCHECK("arg32", cmds[2].args[2])



	BCHECK(!cmds[3].filled)
}

void test_is_blocking_call(void) {
	char blocking_call[] = "cmd 1 2 3  ";
	BCHECK(is_blocking_call(blocking_call))

	char non_blocking_call[] = "cmd 1 2 3 & ";
	BCHECK(!is_blocking_call(non_blocking_call))
	BCHECK(10 == strlen(non_blocking_call))
}





int main(void) {
	test_get_part_type();
	test_split();
	test_get_command();
	test_get_commands();
	test_is_blocking_call();


	printf("--------------%s\n", ok ? "--" : "----");
	printf("TOTAL RESULT: %s\n", ok ? "OK" : "FAIL");
	return EXIT_SUCCESS;
}
#endif
