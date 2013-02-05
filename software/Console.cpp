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

#include <QtExtSerialPort/qextserialport.h>

/* LOCAL INCLUDES */

#include "Console.h"
#include "main.h"

/**
 * Console widget with display area and line input.
 *
 * @param parent the parent widget.
 */
Console::Console(QWidget *parent) : QWidget(parent) {

	// Setup Serial Port - 230400 baud 8 bits 1 stop no parity no flow ctrl.
	_port = new QextSerialPort("/usr/ttyUSB0", QextSerialPort::EventDriven);
	_port->setBaudRate(BAUD230400);
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
}

/**
 * Handle an enter press or send button click causing the data in the input line to be sent
 */
void Console::doSend(void) {

	if (++_num_lines > 1000) {

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

	_output->append(_input->text());
	_input->setText(tr(""));
}

Console::~Console() {

	delete _port;
}
