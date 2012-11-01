/*
 * va2epr-tnc software - terminal node controller software for va2epr-tnc
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

/* GLOBAL INCLUDES */

#include <QtGui>

/* LOCAL INCLUDES */

#include "Console.h"
#include "main.h"

Console::Console(QWidget *parent) : QWidget(parent) {

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

	_send = new QPushButton();
	_send->setText(tr("Send"));
	connect(_send, SIGNAL(clicked()), this, SLOT(doSend()));
	_inputLayout->addWidget(_send);

	_layout->addLayout(_inputLayout);

	setLayout(_layout);
}

void Console::doSend(void) {

	_output->append(_input->text());
	_input->setText(tr(""));
}
