/*

NAME
vs - prints sin(x) and cos(x) of command line argument x (in radians)
SYNOPSIS
a x
DESCRIPTION
Prints sin(x) and cos(x)
using sin207 and cos207 functions
from different minor versions of libtri207.

*/

#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void
print_sin_cos(const double x, const char * const library, const char * const version)
{

    void *lib_handle;

    typedef double (*fun)(double);

    void *cos_handle;
    void *sin_handle;

    const char *error;

    lib_handle = dlopen(library, RTLD_LAZY);

    if (! lib_handle) {
        printf("DLOPEN (%s) error: %s.\n", dlerror(), library);
        return;
    }

    (void) dlerror();  // Discard returned value.

    if (version == nullptr) {
        cos_handle = dlsym(lib_handle, "cos207");
        sin_handle = dlsym(lib_handle, "sin207");
    } else {
        cos_handle = dlvsym(lib_handle, "cos207", version);
        sin_handle = dlvsym(lib_handle, "sin207", version);
    }

    error = dlerror();

    if (error != nullptr) {
        printf("DLSYM (%s, %s) error: %s.\n", library, version, error);
        return;
    }

    printf("cos(%f) = %f (%s, %s).\n", x, ((fun) cos_handle)(x), library, version);
    printf("sin(%f) = %f (%s, %s).\n", x, ((fun) sin_handle)(x), library, version);

    dlclose(lib_handle);

}

int
main(int argc, char *argv[])
{

    if (argc != 2) {
        printf("Usage: %s x (radians)\n", argv[0]);
        return EXIT_FAILURE;
    }

    //

    double x = strtod(argv[1], nullptr);

    // 1.3.0

    print_sin_cos(x, "./1.3.0/libtri207.so", nullptr);
    print_sin_cos(x, "./1.3.0/libtri207.so", "VER_1_3_0");
    print_sin_cos(x, "./1.3.0/libtri207.so", "VER_1_4_0");

    // 1.4.0

    print_sin_cos(x, "./1.4.0/libtri207.so", nullptr);
    print_sin_cos(x, "./1.4.0/libtri207.so", "VER_1_3_0");
    print_sin_cos(x, "./1.4.0/libtri207.so", "VER_1_4_0");

    return EXIT_SUCCESS;

}
