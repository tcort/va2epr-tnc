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

/* GLOBAL INCLUDES */

#include <QtGui>

#include <QtExtSerialPort/qextserialport.h>

/* LOCAL INCLUDES */

#include "Console.h"
#include "main.h"

#include "va2epr_tnc.h"

/**
 * Console widget with display area and line input.
 *
 * @param parent the parent widget.
 */
Console::Console(QWidget *parent) : QWidget(parent) {

	qDebug() << "Console::Console() Enter";

	_parent = parent;

	// Setup Serial Port - 57600 baud 8 bits 1 stop no parity no flow ctrl.
	_port = new QextSerialPort("/dev/ttyUSB0", QextSerialPort::EventDriven);
	_port->setBaudRate(BAUD57600);
	_port->setFlowControl(FLOW_OFF);
	_port->setParity(PAR_NONE);
	_port->setDataBits(DATA_8);
	_port->setStopBits(STOP_1);

	_num_lines = 0;

	_layout = new QVBoxLayout();
	_inputLayout = new QHBoxLayout();

	_output = new QTextEdit();
	_output->setReadOnly(true);
	_output->setFontFamily("Courier");
	_output->setFontPointSize(12.0);
	_output->setFontWeight(QFont::DemiBold);

	QPalette palette = _output->palette();
	palette.setColor(QPalette::Base, Qt::black);
	palette.setColor(QPalette::Text, Qt::cyan);
	_output->setPalette(palette);

	_layout->addWidget(_output);

	_input = new QLineEdit();
	_input->setFont(QFont("Courier", 12.0, QFont::DemiBold));
	_input->setPalette(palette);
	connect(_input, SIGNAL(returnPressed()), this, SLOT(doSend()));
	_inputLayout->addWidget(_input);

	_send = new QPushButton(QIcon(":/icons/actions/mail-reply-sender.svg"), tr("Send"));
	connect(_send, SIGNAL(clicked()), this, SLOT(doSend()));
	_inputLayout->addWidget(_send);

	_layout->addLayout(_inputLayout);

	setLayout(_layout);

	_recvBuf = "";

	qDebug() << "Console::Console() Complete";
}

/**
 * Write to the serial port.
 * @param s string to write
 * @return true if string written OK
 */
bool Console::writePort(QString s) {

	qDebug() << "Console::writePort() Enter";

	bool rc;

	if (_port->isOpen()) {

		rc = (_port->write(s.toLatin1()) == s.toLatin1().length());

	} else {

		rc = false;
		qDebug() << "Console::writePort() Port is closed";
	}

	qDebug() << "Console::writePort() Complete";
	return rc;
}

/**
 * Handle an enter press or send button click causing the data in the input line to be sent
 */
void Console::doSend(void) {

	qDebug() << "Console::doSend() Enter";

	if (this->writePort(_input->text())) {
		this->append(_input->text());
		_input->setText(tr(""));
		qDebug() << "Console::doSend() write OK";
	} else {
		qDebug() << "Console::doSend() write FAIL";
	}

	qDebug() << "Console::doSend() Complete";
}

/**
 * Append text to the output area and manages the scroll buffer.
 * @param s string to write to the output area.
 */
void Console::append(QString s) {

	qDebug() << "Console::append() Enter";

	if (++_num_lines > 100) {

		// Remove 1st line of text, then move the line below up.
		QTextCursor tc = _output->textCursor();
		tc.movePosition(QTextCursor::Start);
		tc.select(QTextCursor::LineUnderCursor);
		tc.removeSelectedText();
		tc.movePosition(QTextCursor::Down);
		tc.deletePreviousChar();
		tc.movePosition(QTextCursor::End);

		_num_lines--;
	}

	_output->append(s);

	qDebug() << "Console::append() Complete";
}


/**
 * When data is ready to be read, read it.
 */
void Console::doRecv() {

	_outputLock.lock();

	QByteArray bytes;
	int bytesAvailable = _port->bytesAvailable();
	bytes.resize(bytesAvailable);
	_port->read(bytes.data(), bytes.size());

	_recvBuf += bytes;

	printf("%lld\n", QDateTime::currentMSecsSinceEpoch());
	qDebug(" Got [" + bytes + "]");

	if (_recvBuf.contains("}")) {
		((va2epr_tnc *) (this->_parent))->processMessage(_recvBuf);
		_recvBuf = "";
	}

	_outputLock.unlock();
}

/**
 * Open the Serial Port.
 * @param portName name of the port (example "/dev/ttyUSB0").
 * @param portSpeed baud rate for the port.
 * @return successfulness of the operation (false == fail).
 */
bool Console::openPort(QString portName, QString portSpeed) {

	qDebug() << "Console::openPort() Enter";
	BaudRateType baudRate = (BaudRateType) portSpeed.toInt();

	_port->setPortName(portName.toLatin1());
	_port->setBaudRate(baudRate);

	_port->open(QIODevice::ReadWrite);
	if (_port->isOpen()) {

		qDebug() << "Port Opened OK\n";
		_send->setEnabled(true);
		_input->setEnabled(true);
		connect(_port, SIGNAL(readyRead()), this, SLOT(doRecv()));

		qDebug() << "Console::openPort() Complete";
		return true;
	} else {

		qDebug() << "Failed to Open Port\n";
		qDebug() << "Console::openPort() Complete";
		return false;
	}
}

/**
 * Close the serial port
 * @return successfulness of the operation (false == fail)
 */
bool Console::closePort() {

	qDebug() << "Console::closePort() Enter";

	if (_port->isOpen()) {

		_input->setEnabled(false);
		_send->setEnabled(false);
		disconnect(_port, SIGNAL(readyRead()), this, SLOT(doRecv()));
		_port->close();

	} else {

		qDebug() << "Console::closePort() Port was not Open";
	}

	qDebug() << "Console::closePort() Complete";

	return !(_port->isOpen());
}

/**
 * Sets the port to QextSerialPort::Polling or QextSerialPort::EventDriven
 *
 * @param mode the mode for the serial port.
 */
void Console::setPortMode(QextSerialPort::QueryMode mode) {

	qDebug() << "Console::setPortMode() Enter";
	_port->setQueryMode(mode);
	qDebug() << "Console::setPortMode() Complete";
}

/**
 * Read data from the serial port.
 *
 * @return a string of bytes from the serial port.
 */
QString Console::readPort() {

	qDebug("Console::readPort() Enter");

	QByteArray bytes;
	int bytesAvailable = _port->bytesAvailable();
	bytes.resize(bytesAvailable);
	_port->read(bytes.data(), bytes.size());

	qDebug("Console::readPort() Complete");

	return bytes;
}

/**
 * Deconstruct the COnsole and perform clean up.
 * Closes serial port if it is open.
 */
Console::~Console() {

	qDebug() << "Console::~Console() Enter";

	if (_port->isOpen()) {

		closePort();
	}

	delete _port;

	qDebug() << "Console::~Console() Complete";
}
