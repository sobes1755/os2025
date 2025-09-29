/*

NAME
mktime - TODO
SYNOPSIS
mktime -Y 2025 -M 9 -D 29 -h 12 -m 0 -s 0
DESCRIPTION
TODO

*/

#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

extern int optind, opterr, optopt;
extern char *optarg;

int
main(int argc, char *argv[])
{

	struct tm tm = { .tm_isdst = -1, };

	int opt;

	while ((opt = getopt(argc, argv, ":Y:M:D:h:m:s:")) != -1) {

		switch (opt) {
		case 'Y':
			tm.tm_year = strtol(optarg, nullptr, 10) - 1900;
			break;
		case 'M':
			tm.tm_mon = strtol(optarg, nullptr, 10) - 1;
			break;
		case 'D':
			tm.tm_mday = strtol(optarg, nullptr, 10);
			break;
		case 'h':
			tm.tm_hour = strtol(optarg, nullptr, 10);
			break;
		case 'm':
			tm.tm_min = strtol(optarg, nullptr, 10);
			break;
		case 's':
			tm.tm_sec = strtol(optarg, nullptr, 10);
			break;
        default:
			fprintf(stderr, "mktime error: unrecognized option (%c)!\n", optopt);
			return EXIT_FAILURE;
        }

	}

	printf("Input:\n\n");

	printf("tm_year = %d,\n", tm.tm_year);
	printf("tm_mon = %d,\n", tm.tm_mon);
	printf("tm_mday = %d,\n", tm.tm_mday);
	printf("tm_hour = %d,\n", tm.tm_hour);
	printf("tm_min = %d,\n", tm.tm_min);
	printf("tm_sec = %d,\n", tm.tm_sec);
	printf("tm_sdst = %d.\n\n", tm.tm_isdst);

	time_t t = mktime(&tm);

	printf("Output:\n\n");

	printf("tm_year = %d,\n", tm.tm_year);
	printf("tm_yday = %d,\n", tm.tm_yday);
	printf("tm_mon = %d,\n", tm.tm_mon);
	printf("tm_mday = %d,\n", tm.tm_mday);
	printf("tm_wday = %d,\n", tm.tm_wday);
	printf("tm_hour = %d,\n", tm.tm_hour);
	printf("tm_min = %d,\n", tm.tm_min);
	printf("tm_sec = %d,\n", tm.tm_sec);
	printf("tm_sdst = %d,\n\n", tm.tm_isdst);

	printf("mktime = %ld,\n\n", (long) t);

	if (setlocale(LC_ALL, "") == nullptr) {
		fprintf(stderr, "mktime setlocale error!\n");
	}	
	
	char *asctm = asctime(&tm);
	char str[256];
	strftime(str, sizeof(str), "%c", &tm);

	printf("asctime = %.*s,\n", (int) (strlen(asctm) - 1), asctm);
	printf("strftime = %s,\n\n", str);

	struct tm *gmp = gmtime(&t);
	char *ascgm = asctime(gmp);
	
	printf("asctime (gm) = %.*s.\n", (int) (strlen(ascgm) - 1), ascgm);

	return EXIT_SUCCESS;

}

