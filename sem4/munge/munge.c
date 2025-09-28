/*

NAME
munge - negates uints with different sizes and offsets
SYNOPSIS
munge
DESCRIPTION
Negates a large heap of unsigned integers and
investigates the dependence of average time of execution
on the size of unsigned integers and the offset of the heap.
This code may be regarded as a companion
to the article about alignment:
https://developer.ibm.com/articles/pa-dalign/.

*/

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum
{
	DATA_SIZE = 100 * 1024 * 1024,
	OFFSET_SIZE = 8,
	BITE_SIZE = 4,
	RUNS = 2048,
};

void
munge_8_bits_per_bite(void *data, size_t size)
{
	uint8_t *ptr = data;
	uint8_t *endptr = ptr + size;

	while (ptr != endptr) {
		*ptr = -*ptr;
		++ptr;
	}
}

void
munge_16_bits_per_bite(void *data, size_t size)
{
	uint16_t *ptr = data;
	uint16_t *endptr = ptr + (size >> 1);

	while (ptr != endptr) {
		*ptr = -*ptr;
		++ptr;
	}
}

void
munge_32_bits_per_bite(void *data, size_t size)
{
	uint32_t *ptr = data;
	uint32_t *endptr = ptr + (size >> 2);

	while (ptr != endptr) {
		*ptr = -*ptr;
		++ptr;
	}
}

void
munge_64_bits_per_bite(void *data, size_t size)
{
	uint64_t *ptr = data;
	uint64_t *endptr = ptr + (size >> 3);

	while (ptr != endptr) {
		*ptr = -*ptr;
		++ptr;
	}
}


int
main(void)
{

	char *data = malloc(DATA_SIZE + OFFSET_SIZE);

	if (! data) {
		return EXIT_FAILURE;
	}

	srand(time(nullptr));
	
	for (size_t k = 0; k < DATA_SIZE + OFFSET_SIZE; k ++)
	{
		*(data + k) = rand() % (1 << CHAR_BIT);
	}

	printf("data size = %d MiB, data address = %p\n", DATA_SIZE >> 20, data);
	fflush(stdout);

	clock_t w[BITE_SIZE][OFFSET_SIZE] = {};		// Stopwatch
	size_t r[BITE_SIZE][OFFSET_SIZE] = {};		// Runs

	for (int run = 1; run <= RUNS; ++run) {

		int bite = rand() % BITE_SIZE;			// Choose random size of uint
		int offset = rand() % OFFSET_SIZE;		// Choose random offset of uint heap

		w[bite][offset] -= clock();

		switch (1 << (3 + bite)) {
		case 8:
			munge_8_bits_per_bite(data + offset, DATA_SIZE);
			break;
		case 16:
			munge_16_bits_per_bite(data + offset, DATA_SIZE);
			break;
		case 32:
			munge_32_bits_per_bite(data + offset, DATA_SIZE);
			break;
		case 64:
			munge_64_bits_per_bite(data + offset, DATA_SIZE);
			break;
		}

		w[bite][offset] += clock();
		r[bite][offset] ++;

		printf("current runtime in milliseconds\n\n");
		printf("run = %4d of %d, bits = %2d, offset = %1d, runtime = %10.6f\n\n",
			run, RUNS,
			8 * bite,
			offset,
			(double) w[bite][offset] / r[bite][offset] / CLOCKS_PER_SEC * 1000);

		printf("average runtime in milliseconds\n\n");
		printf("bits ");
		for (int offset = 0; offset < OFFSET_SIZE; ++offset)
			printf("  offset %1d ", offset);
		printf("\n");
			
		for (int bite = 0; bite < BITE_SIZE; ++ bite) {
			printf("%4d ", 8 << bite);
			for (int offset = 0; offset < OFFSET_SIZE; ++offset)
				printf("%10.6f ", r[bite][offset] > 0 ?
					(double) w[bite][offset] / r[bite][offset] / CLOCKS_PER_SEC * 1000 : -1);
			printf("\n");
		}
		printf("\n");

	}

	free(data);

	return EXIT_SUCCESS;

}

