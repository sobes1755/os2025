/*

NAME
a - prints sin and cos of command line argument x (in radians)
SYNOPSIS
a x

*/

#include "tri207.h"

#include <stdio.h>
#include <stdlib.h>

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
