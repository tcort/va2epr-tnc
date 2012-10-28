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

#include <QtGui>
#include <QDesktopServices>
#include <QCoreApplication>


#include "AboutDialog.h"
#include "main.h"
#include "va2epr_tnc.h"

va2epr_tnc::va2epr_tnc(void) {

	_widget = new QWidget();
	setCentralWidget(_widget);

	_quitAction = new QAction(tr("&Quit"), this);
	_quitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));
	connect(_quitAction, SIGNAL(triggered()), this, SLOT(close()));

	_file = menuBar()->addMenu(tr("&File"));
	_file->addAction(_quitAction);

	_helpAction = new QAction(tr("&About"), this);
	connect(_helpAction, SIGNAL(triggered()), this, SLOT(doAbout()));

	_help = menuBar()->addMenu(tr("&Help"));
	_help->addAction(_helpAction);

	QLabel *testLabel = new QLabel();
	testLabel->setSizePolicy(QSizePolicy::Expanding,
                                    QSizePolicy::Expanding);
	testLabel->setAlignment(Qt::AlignCenter);
	testLabel->setMinimumSize(240, 160);
	testLabel->setText("test");

	_layout = new QVBoxLayout();
	_layout->addWidget(testLabel);
	_widget->setLayout(_layout);

	setWindowTitle(tr("%1").arg(PROGRAM_NAME));
	resize(300, 200);
}

void va2epr_tnc::doAbout(void) {

	AboutDialog about(this);
	about.exec();
}

va2epr_tnc::~va2epr_tnc(void) {

}
