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

#ifndef __CONNECT_DIALOG_H
#define __CONNECT_DIALOG_H

#include <QtGui>
#include <QObject>

#include <QtExtSerialPort/qextserialenumerator.h>

/**
 * Connection Settings
 */
class ConnectDialog : public QDialog {
	Q_OBJECT

	public:
		/**
		 * Connection Settings
		 */
		ConnectDialog(QWidget * parent = 0);

		QString getPort();
		QString getSpeed();

	private:
		QextSerialEnumerator *_portList;		
		QComboBox *_portSelect;
		QComboBox *_speedSelect;
};

#endif
