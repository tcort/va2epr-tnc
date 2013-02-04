/*
 * va2epr-tnc software - terminal node controller software for va2epr-tnc
 * Copyright (C) 2012, 2013 Thomas Cort <va2epr@rac.ca>
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <QString>
#include "ReaderThread.h"

void ReaderThread::run(void) {

	int rc;
	const char str[2] = { '\0', '\0' };

	do {

		rc = ::read(_fd, (void *) str, 1);
		if (rc == 1) {
			_output_lock->lock();
			_output->append(QString(str));
			_output_lock->unlock();
		}

	} while (1);
}
