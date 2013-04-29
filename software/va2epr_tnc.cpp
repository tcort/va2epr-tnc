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

#include <QtGui>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QtWebKit>

#include "AboutDialog.h"
#include "ConnectDialog.h"
#include "Console.h"
#include "main.h"
#include "Settings.h"
#include "va2epr_tnc.h"

/**
 * Main TNC Window
 */
va2epr_tnc::va2epr_tnc(void) {

	qDebug() << "va2epr_tnc::va2epr_tnc() Enter";

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

	_console = new Console(this);
	_settings = new Settings(_console); // share the console with Settings so it can access the serial port

	_tabs = new QTabWidget();
	_tabs->setTabPosition(QTabWidget::South);
	_tabs->addTab(_console, QIcon(":/icons/apps/utilities-terminal.svg"), tr("Console"));
	_tabs->addTab(_www, QIcon(":/icons/apps/internet-web-browser.svg"), tr("Map"));
	_tabs->addTab(_settings, QIcon(":/icons/categories/preferences-system.svg"), tr("Settings"));

	_status = new QLabel();
	_status->setText(tr("Disconnected"));

	_tabs->setTabEnabled(2, false);
	statusBar()->addPermanentWidget(_status, 1);

	_layout = new QVBoxLayout();
	_layout->addWidget(_tabs);
	_widget->setLayout(_layout);

	setWindowTitle(tr("%1").arg(PROGRAM_NAME));
	setWindowIcon(QIcon(":/icons/devices/network-wireless.svg"));
	resize(800, 600);

	doDisconnect();

	this->show();
	_www->show();

	qDebug() << "va2epr_tnc::va2epr_tnc() Complete";
}

/*
 * Put a marker on the map.
 *
 * @param lon longitude
 * @param lat latitude
 */
void va2epr_tnc::addMarker(QString lon, QString lat) {
	_www->page()->mainFrame()->evaluateJavaScript("addMarker(" + lon + ", " + lat + "); null");
}

/*
 * Convert Degrees/Minutes to decimal degrees.
 *
 * For example, "45 15.000" becomes "45.25000"
 *
 * @param dm coordinate in degrees minutes notation.
 * @return coordinate in decimal degrees notation.
 */
QString va2epr_tnc::dmToD(QString dm) {

	double d = 0.0;

	char direction = dm.at(0).toAscii();
	QString part1 = dm.mid(dm.indexOf(' ') + 1);
	QString hour = part1.mid(0, part1.indexOf(' '));
	QString minutes = part1.mid(part1.indexOf(' ') + 1);

	d = hour.toDouble() + (minutes.toDouble()/60.0);
	if (direction == 'S' || direction == 'W') {
		d = d * -1.0;
	}

	return QString("%1").arg(d, 0, 'f');
}

/**
 * Process an incoming message.
 *
 * @param str string containing 1 or more messages {m:CALLSIGN@lon,lat|crch,crcl}
 */
void va2epr_tnc::processMessage(QString str) {

	QRegExp rx("\\{(m:[a-zA-Z0-9]+@[WE] [0-9]+ [0-9]+\\.[0-9]+,[NS] [0-9]+ [0-9]+\\.[0-9]+\\|\\d+,\\d+)\\}");
	QStringList list;
	QString msg;

	int pos = 0;
	while ((pos = rx.indexIn(str, pos)) != -1)
	{
		list << rx.cap(1);
		pos += rx.matchedLength();
	}

	foreach (msg, list) {
		qWarning() << msg;

		QString callsign = msg.mid(msg.indexOf(':') + 1, msg.indexOf('@') - msg.indexOf(':') - 1);

		QString coords = msg.mid(msg.indexOf('@') + 1, msg.indexOf('|') - msg.indexOf('@') - 1);
		QString longitude = coords.mid(0, coords.indexOf(','));
		QString latitude = coords.mid(coords.indexOf(',') + 1);

		QString crc = msg.mid(msg.indexOf('|') + 1);
		QString crch = crc.mid(0, crc.indexOf(','));
		QString crcl = crc.mid(crc.indexOf(',') + 1);

		// TODO check crc values here.
		// If it passed the QRegEx, it's likely good

		qWarning() << callsign;
		qWarning() << coords;
		qWarning() << longitude;
		qWarning() << dmToD(longitude);
		qWarning() << latitude;
		qWarning() << dmToD(latitude);

		qWarning() << crch;
		qWarning() << crcl;

		_console->append("{" + msg + "}");

		addMarker(dmToD(longitude), dmToD(latitude));
	}
}

/**
 * Process clicks of the about menu item
 */
void va2epr_tnc::doAbout(void) {

	qDebug() << "va2epr_tnc::doAbout() Enter";

	AboutDialog about(this);
	about.exec();

	qDebug() << "va2epr_tnc::doAbout() Complete";
}

/**
 * Process clicks of the connect button
 */
void va2epr_tnc::doConnect(void) {

	qDebug() << "va2epr_tnc::doConnect() Enter";

	bool rc;
	ConnectDialog cd;

	cd.exec();

	rc = _console->openPort(cd.getPort(), cd.getSpeed());
	if (rc) {
		_toolbarConnectAction->setEnabled(false);
		_toolbarDisconnectAction->setEnabled(true);
		_status->setText(tr("Connected"));
		_tabs->setTabEnabled(2, true);
	}

	qDebug() << "va2epr_tnc::doConnect() Complete";
}

/**
 * Process clicks of the disconnect button
 */
void va2epr_tnc::doDisconnect(void) {

	qDebug() << "va2epr_tnc::doDisconnect() Enter";

	_toolbarConnectAction->setEnabled(true);
	_toolbarDisconnectAction->setEnabled(false);
	_status->setText(tr("Disconnected"));

	_tabs->setTabEnabled(2, false);
	if(_console->isPortOpen()) {

		_console->closePort();
	}

	qDebug() << "va2epr_tnc::doDisconnect() Complete";
}

/**
 * Destructor
 */
va2epr_tnc::~va2epr_tnc(void) {

	qDebug() << "va2epr_tnc::~va2epr_tnc() Enter";
	qDebug() << "va2epr_tnc::~va2epr_tnc() Complete";
}

