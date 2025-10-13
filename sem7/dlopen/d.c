/*

NAME
d - prints sin and cos of command line argument x (in radians)
SYNOPSIS
d x
DESCRIPTION
Prints sin(x) and cos(x) twice:
1) using sin and cos from linm,
2) using sin207 and cos207 from libtri207.

*/

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{

    if (argc != 2) {
        printf("Usage: %s x (radians)\n", argv[0]);
        return EXIT_FAILURE;
    }

    //

    double x = strtod(argv[1], nullptr);

    void *lib_handle;

    typedef double (*fun)(double);

    void *cos_handle;
    void *sin_handle;

    const char *error;

    // libm

    lib_handle = dlopen("libm.so.6", RTLD_LAZY);

    if (! lib_handle) {
        printf("DLOPEN error: %s.\n", dlerror());
        return EXIT_FAILURE;
    }

    (void) dlerror();  // Discard returned value.

    cos_handle = dlsym(lib_handle, "cos");
    sin_handle = dlsym(lib_handle, "sin");

    error = dlerror();

    if (error != nullptr) {
        printf("DLSYM error: %s.\n", error);
        return EXIT_FAILURE;
    }

    printf("cos(%f) = %f.\n", x, ((fun) cos_handle)(x));
    printf("sin(%f) = %f.\n", x, ((fun) sin_handle)(x));

    dlclose(lib_handle);

    // libtri207

    lib_handle = dlopen("./libtri207.so", RTLD_LAZY);

    if (! lib_handle) {
        printf("DLOPEN error: %s.\n", dlerror());
        return EXIT_FAILURE;
    }

    (void) dlerror();

    cos_handle = dlsym(lib_handle, "cos207");
    sin_handle = dlsym(lib_handle, "sin207");

    error = dlerror();

    if (error != nullptr) {
        printf("DLSYM error: %s.\n", error);
        return EXIT_FAILURE;
    }

    printf("cos207(%f) = %f.\n", x, ((fun) cos_handle)(x));
    printf("sin207(%f) = %f.\n", x, ((fun) sin_handle)(x));

    dlclose(lib_handle);

    return EXIT_SUCCESS;

}
