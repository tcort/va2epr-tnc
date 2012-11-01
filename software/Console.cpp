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

	QVBoxLayout *layout = new QVBoxLayout(this);

	QLabel *testLabel = new QLabel(tr("Insert Console Here"));

	QFont font = testLabel->font();
	font.setPointSize(font.pointSize() + 4);
	font.setWeight(QFont::Bold);
	testLabel->setFont(font);
	testLabel->setMargin(4);
	testLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(testLabel);

	setLayout(layout);
}
