/*
 * va2epr-tnc firmware tools - firmware related utilities for va2epr-tnc
 * Copyright (C) 2012 Thomas Cort <va2epr@rac.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <math.h>

/* Default size of sinewave value table */
#define NSAMPLES 32

/* Define LSB_FIRST for a Rev B board (least significant bit first) */
#define LSB_FIRST

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

#ifdef LSB_FIRST
		unsigned char j;
		unsigned char wave_fixed = 0;

		for (j = 0; j < 8; j++) {
			wave_fixed |= ((wave >> (7-j)) & 0x01) << (j);
		}

		wave = wave_fixed;
#endif

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
