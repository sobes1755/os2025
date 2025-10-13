/*

NAME
b - prints sin^2(x)+cos^2(x) of command line argument x (in radians)
SYNOPSIS
b x

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

    printf("sin207(%f)^2 + cos207(%f)^2 = %f.\n",
        x, x,
        sin207(x)*sin207(x)+cos207(x)*cos207(x));

    return EXIT_SUCCESS;

}

