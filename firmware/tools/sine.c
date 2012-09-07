#include <stdio.h>
#include <math.h>

/* Default size of sinewave value table */
#define NSAMPLES 16

/*
 * Generate a table of sinewave values
 */
int main(int argc, char *argv[]) {

	int i, nsamples;
	float nsamples_f;

	if (argc == 2 && atoi(argv[1]) >= 4 && atoi(argv[1]) <= 256) {
		nsamples = atoi(argv[1]);
	} else {
		nsamples = NSAMPLES;
	}

	nsamples_f = 1.0 * nsamples;

	printf("unsigned char sinewave_index = 0;\n");
	printf("unsigned char sinewave[%d] = {\n\t", nsamples);

	for (i = 0; i < nsamples; i++) {

		double sine = sin(((i*1.0)/nsamples_f) * (M_PI*2.0));
		unsigned char wave = (sine + 1.0) * 127.5;

		printf("%3d", wave);

		if (i < nsamples - 1) {
			printf(", ");
		} else {
			break;
		}

		if ((i + 1) % 8 == 0) {
			printf("\n\t");
		}
	}

	printf("\n};\n");

	return 0;
}
