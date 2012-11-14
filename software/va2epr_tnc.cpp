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

#ifdef __linux
#include "LinuxSerialPort.h"
#endif

#ifdef _WIN32
#include "WindowsSerialPort.h"
#endif

va2epr_tnc::va2epr_tnc(void) {

#ifdef __linux
	_serial = new LinuxSerialPort();
#endif

#ifdef _WIN32
	_serial = new LinuxSerialPort();
#endif

	_widget = new QWidget();
	setCentralWidget(_widget);

	_quitAction = new QAction(tr("&Quit"), this);
	_quitAction->setIcon(QIcon(":/icons/actions/system-log-out.svg"));
	_quitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));
	connect(_quitAction, SIGNAL(triggered()), this, SLOT(close()));

	_file = menuBar()->addMenu(tr("&File"));
	_file->addAction(_quitAction);

	_helpAction = new QAction(tr("&About %1").arg(PROGRAM_NAME), this);
	_helpAction->setIcon(QIcon(":/icons/status/dialog-information.svg"));
	connect(_helpAction, SIGNAL(triggered()), this, SLOT(doAbout()));

	_help = menuBar()->addMenu(tr("&Help"));
	_help->addAction(_helpAction);

	_toolbar = addToolBar("_toolbar");

	_toolbarConnectAction = _toolbar->addAction(QIcon(":/icons/status/network-idle.svg"), tr("Connect"));
	connect(_toolbarConnectAction, SIGNAL(triggered()), this, SLOT(doConnect()));

	_toolbarDisconnectAction = _toolbar->addAction(QIcon(":/icons/status/network-offline.svg"), tr("Disconnect"));
	_toolbarDisconnectAction->setEnabled(false);
	connect(_toolbarDisconnectAction, SIGNAL(triggered()), this, SLOT(doDisconnect()));

	_toolbar->addSeparator();

	_toolbarQuitAction = _toolbar->addAction(QIcon(":/icons/actions/system-log-out.svg"), tr("Quit"));
	connect(_toolbarQuitAction, SIGNAL(triggered()), this, SLOT(close()));

	_www = new QWebView();
	_www->load(QUrl("qrc:/html/va2epr-tnc.html"));
	_www->show();

	// TODO here's a reminder how to call JS functions from C++
	// I'll need this for later when I add/remove markers
	// _www->page()->mainFrame()->evaluateJavaScript("alert('test'); null");

	_console = new Console();
	_settings = new Settings();

	_tabs = new QTabWidget();
	_tabs->setTabPosition(QTabWidget::South);
	_tabs->addTab(_console, QIcon(":/icons/apps/utilities-terminal.svg"), tr("Console"));
	_tabs->addTab(_www, QIcon(":/icons/apps/internet-web-browser.svg"), tr("Map"));
	_tabs->addTab(_settings, QIcon(":/icons/categories/preferences-system.svg"), tr("Settings"));

	_status = new QLabel();
	_status->setText(tr("Disconnected"));
	// TODO call doDisconnect here

	statusBar()->addPermanentWidget(_status, 1);

	_layout = new QVBoxLayout();
	_layout->addWidget(_tabs);
	_widget->setLayout(_layout);

	setWindowTitle(tr("%1").arg(PROGRAM_NAME));
	setWindowIcon(QIcon(":/icons/devices/network-wireless.svg"));
	resize(800, 600);

	doDisconnect();
}

void va2epr_tnc::doAbout(void) {

	AboutDialog about(this);
	about.exec();
}

void va2epr_tnc::doConnect(void) {

	if (_serial->isOpen()) {
		_serial->close();
	}

	// Try _serial->open() here

	_toolbarConnectAction->setEnabled(false);
	_toolbarDisconnectAction->setEnabled(true);
	_status->setText(tr("Connected"));

	// TODO: enable [Send] button and input area
	// TODO: disable read/program on settings tab
}

void va2epr_tnc::doDisconnect(void) {

	_toolbarConnectAction->setEnabled(true);
	_toolbarDisconnectAction->setEnabled(false);
	_status->setText(tr("Disconnected"));

	if (_serial->isOpen()) {
		_serial->close();
	}

	// TODO: disable [Send] button and input area
	// TODO: disable read/program on settings tab
}

va2epr_tnc::~va2epr_tnc(void) {

	if (_serial->isOpen()) {
		doDisconnect();
	}

	delete _serial;
}
