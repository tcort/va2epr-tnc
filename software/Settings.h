/*
 * va2epr-tnc software - terminal node controller software for va2epr-tnc
 * Copyright (C) 2012, 2013 Thomas Cort <linuxgeek@gmail.com>
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

#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <QtGui>
#include <QObject>

#include "Console.h"

class Settings : public QWidget {

	Q_OBJECT

	public:

		Settings(Console *console, QWidget * parent = 0);

	private slots:

		void doProgram();
		void doRead();

	private:

		Console *_console;

		QFormLayout *_formLayout;
		QHBoxLayout *_buttonLayout;
		QVBoxLayout *_layout;

		QLabel *_tx_delayLabel;
		QLineEdit *_tx_delay;
		QIntValidator *_tx_delay_validator;

		QLabel *_pLabel;
		QLineEdit *_p;
		QIntValidator *_p_validator;

		QLabel *_slot_timeLabel;
		QLineEdit *_slot_time;
		QIntValidator *_slot_time_validator;

		QLabel *_tx_tailLabel;
		QLineEdit *_tx_tail;
		QIntValidator *_tx_tail_validator;

		QLabel *_full_duplexLabel;
		QCheckBox *_full_duplex;

		QLabel *_callsignLabel;
		QLineEdit *_callsign;
		QRegExpValidator *_callsign_validator;

		QPushButton *_program;
		QPushButton *_read;
};

#endif
