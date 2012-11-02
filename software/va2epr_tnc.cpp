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
#include <QtWebKit>

#include "AboutDialog.h"
#include "Console.h"
#include "main.h"
#include "Settings.h"
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

	_www = new QWebView();
	_www->load(QUrl("qrc:/html/va2epr_tnc.html"));
	_www->show();

	// TODO here's a reminder how to call JS functions from C++
	// I'll need this for later when I add/remove markers
	// _www->page()->mainFrame()->evaluateJavaScript("alert('test'); null");

	_console = new Console();
	_settings = new Settings();

	_tabs = new QTabWidget();
	_tabs->setTabPosition(QTabWidget::South);
	_tabs->addTab(_www, tr("Map"));
	_tabs->addTab(_console, tr("Console"));
	_tabs->addTab(_settings, tr("Settings"));

	_status = new QLabel();
	_status->setText(tr("Disconnected"));

	statusBar()->addPermanentWidget(_status, 1);

	_layout = new QVBoxLayout();
	_layout->addWidget(_tabs);
	_widget->setLayout(_layout);

	setWindowTitle(tr("%1").arg(PROGRAM_NAME));
	resize(800, 600);
}

void va2epr_tnc::doAbout(void) {

	AboutDialog about(this);
	about.exec();
}

va2epr_tnc::~va2epr_tnc(void) {

}
