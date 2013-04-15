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

/* GLOBAL INCLUDES */

#include <QtGui>
#include <QtExtSerialPort/qextserialenumerator.h>

/* LOCAL INCLUDES */

#include "ConnectDialog.h"
#include "main.h"

/**
 * Allows the user to choose what serial port / speed to use when connecting.
 *
 * @param parent the parent widget
 */
ConnectDialog::ConnectDialog(QWidget *parent) : QDialog(parent, Qt::Dialog) {

	int i;
	qDebug() << "ConnectDialog::ConnectDialog() Enter";

	setWindowTitle(tr("Connect"));

	QVBoxLayout *layout = new QVBoxLayout();

	_portList = new QextSerialEnumerator(this);
	_portSelect = new QComboBox();
	QList<QextPortInfo> portInfos = _portList->getPorts();
	QextPortInfo portInfo;

	QHBoxLayout *deviceLayout = new QHBoxLayout();
        QLabel *device = new QLabel(tr("Device"));

	deviceLayout->addWidget(device);

	// Almost always on /dev/ttyUSB0 on Linux systems
	i = 0;
	foreach(portInfo, portInfos) {
		_portSelect->addItem(portInfo.portName);
		if (!strcmp(portInfo.portName.toStdString().c_str(), "/dev/ttyUSB0")) {
			_portSelect->setCurrentIndex(i);
			break;
		}
		i++;
	}
	deviceLayout->addWidget(_portSelect);
	layout->addLayout(deviceLayout);

	QHBoxLayout *speedLayout = new QHBoxLayout();
        QLabel *speed = new QLabel(tr("Baud Rate"));
	speedLayout->addWidget(speed);

	_speedSelect = new QComboBox();
	_speedSelect->addItem(tr("110"));
	_speedSelect->addItem(tr("300"));
	_speedSelect->addItem(tr("600"));
	_speedSelect->addItem(tr("1200"));
	_speedSelect->addItem(tr("2400"));
	_speedSelect->addItem(tr("4800"));
	_speedSelect->addItem(tr("9600"));
	_speedSelect->addItem(tr("19200"));
	_speedSelect->addItem(tr("38400"));
	_speedSelect->addItem(tr("57600"));
	_speedSelect->addItem(tr("115200"));
	_speedSelect->setCurrentIndex(_speedSelect->count()-2);

	speedLayout->addWidget(_speedSelect);
	layout->addLayout(speedLayout);

	QPushButton *connect = new QPushButton;
	connect->setText(tr("Connect"));
	layout->connect(connect, SIGNAL(clicked()), this, SLOT(close()));
	layout->addWidget(connect);

	setLayout(layout);

	qDebug() << "ConnectDialog::ConnectDialog() Complete";
}

QString ConnectDialog::getPort() {

	return _portSelect->currentText();
}

QString ConnectDialog::getSpeed() {

	return _speedSelect->currentText();
}
