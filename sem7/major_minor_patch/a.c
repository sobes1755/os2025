/*

NAME
a - prints sin(x) and cos(x) of command line argument x (in radians)
SYNOPSIS
a x
DESCRIPTION
Prints sin(x) and cos(x)
using sin207 and cos207 functions from libtri207.

*/

#include <stdio.h>
#include <stdlib.h>

double sin207(double);
double cos207(double);

int
main(int argc, char *argv[])
{

    if (argc != 2) {
        printf("Usage: %s x (radians)\n", argv[0]);
        return EXIT_FAILURE;
    }

    double x = strtod(argv[1], nullptr);

    printf("sin207(%f) = %f.\n", x, sin207(x));
    printf("cos207(%f) = %f.\n", x, cos207(x));

    return EXIT_SUCCESS;

}
