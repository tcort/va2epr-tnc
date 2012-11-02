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

#include "Settings.h"
#include "main.h"

Settings::Settings(QWidget *parent) : QWidget(parent) {

	_layout = new QVBoxLayout();

	_program = new QPushButton();
	_program->setText(tr("Program"));
	connect(_program, SIGNAL(clicked()), this, SLOT(doProgram()));
	_layout->addWidget(_program);

	setLayout(_layout);
}

void Settings::doProgram(void) {

}
