#include <minix/syslib.h>
#include <minix/drivers.h>

#include "keyboard.h"
#include "test3.h"
#include "i8254.h"

static int proc_args(int argc, char *argv[]);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char *argv[]);



int main(int argc, char **argv){
	/* Initialize service */

	sef_startup();

	printf("lab3: The PC's Keyboard  \n");
	if ( argc == 1) {
		print_usage(argv);
		return 0;
	} else {
		proc_args(argc, argv);
	}

}

static void print_usage(char *argv[]) {
	printf("Usage: one of the following:\n"
			"\t service run %s -args \"kbd_test_scan<unsigned short asm>\" \n"
			"\t service run %s -args \"timer_test_square <unsigned long>\n"
			"\t service run %s -args \"timer_test_int <unsigned long>\" \n"
			,argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	char *str;
	long num;
	/* check the function to test: if the first characters match, accept it */
	if (strncmp(argv[1], "kbd_test_scan <unsigned ass> ", strlen("kbd_test_scan")) == 0) {
		if( argc != 3 ) {
			printf("Timer:wrong no of arguments for test of timer_test_config() \n");
			return 1;
		}
		unsigned long ass = parse_ulong(argv[2], 10);
		kbd_test_scan(ass);
		return 0;
	}
	else {
		printf("Timer:non valid function \"%s\" to test\n", argv[1]);
		return 1;
	}
}

static unsigned long parse_ulong(char *str, int base) {
	char *endptr;
	unsigned long val;

	val = strtoul(str, &endptr, base);

	if ((errno == ERANGE && val == ULONG_MAX )
			|| (errno != 0 && val == 0)) {
		perror("strtol");
		return ULONG_MAX;
	}

	if (endptr == str) {
		printf("video_txt: parse_ulong: no digits were found in %s \n", str);
		return ULONG_MAX;
	}

	/* Successful conversion */
	return val;
}
