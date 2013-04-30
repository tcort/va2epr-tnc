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
#include <unistd.h>

/* LOCAL INCLUDES */

#include "Settings.h"
#include "main.h"

/**
 * Settings widget for TNCs
 */
Settings::Settings(Console *console, QWidget *parent) : QWidget(parent) {

	qDebug() << "Settings::Settings() Enter";

	_layout = new QVBoxLayout();
	_buttonLayout = new QHBoxLayout();
	_formLayout = new QFormLayout();

	_tx_delayLabel = new QLabel();
	_tx_delayLabel->setText(tr("TX Delay"));
	_tx_delayLabel->setToolTip(tr("Transmitter Keyup Delay in 10ms Units (Default 50)"));
	_tx_delay = new QLineEdit();
	_tx_delay->setToolTip(tr("Transmitter Keyup Delay in 10ms Units (Default 50)"));
	_tx_delay->setText(tr("10"));
	_tx_delay_validator = new QIntValidator(0, 255);
	_tx_delay->setValidator(_tx_delay_validator);
	_formLayout->addRow(_tx_delayLabel, _tx_delay);

	_pLabel = new QLabel();
	_pLabel->setText(tr("p"));
	_pLabel->setToolTip(tr("Scaled Persistance Paramter (Default 63)"));
	_p = new QLineEdit();
	_p->setToolTip(tr("Scaled Persistance Paramter (Default 63)"));
	_p->setText(tr("63"));
	_p_validator = new QIntValidator(0, 255);
	_p->setValidator(_p_validator);
	_formLayout->addRow(_pLabel, _p);

	_slot_timeLabel = new QLabel();
	_slot_timeLabel->setText(tr("Slot Time"));
	_slot_timeLabel->setToolTip(tr("Slot Interval in 10ms Units (Default 10)"));
	_slot_time = new QLineEdit();
	_slot_time->setToolTip(tr("Slot Interval in 10ms Units (Default 10)"));
	_slot_time->setText(tr("10"));
	_slot_time_validator = new QIntValidator(0, 255);
	_slot_time->setValidator(_slot_time_validator);
	_formLayout->addRow(_slot_timeLabel, _slot_time);

	_tx_tailLabel = new QLabel();
	_tx_tailLabel->setText(tr("TX Tail"));
	_tx_tailLabel->setToolTip(tr("Time to hold after FCS in 10ms Units (Default 50)"));
	_tx_tail = new QLineEdit();
	_tx_tail->setToolTip(tr("Time to hold after FCS in 10ms Units (Default 50)"));
	_tx_tail->setText(tr("50"));
	_tx_tail_validator = new QIntValidator(0, 255);
	_tx_tail->setValidator(_tx_tail_validator);
	_formLayout->addRow(_tx_tailLabel, _tx_tail);

	_full_duplexLabel = new QLabel();
	_full_duplexLabel->setText(tr("Full Duplex"));
	_full_duplexLabel->setToolTip(tr("Unchecked = Half Duplex, Checked = Full Duplex (Default Half Duplex)"));
	_full_duplex = new QCheckBox();
	_full_duplex->setToolTip(tr("Unchecked = Half Duplex, Checked = Full Duplex (Default Half Duplex)"));
	_formLayout->addRow(_full_duplexLabel, _full_duplex);

	_callsignLabel = new QLabel();
	_callsignLabel->setText(tr("Callsign"));
	_callsignLabel->setToolTip(tr("Enter Your Callsign"));
	_callsign = new QLineEdit();
	_callsign->setToolTip(tr("Enter Your Callsign"));
	_callsign->setText(tr("XX0XXX"));
	QRegExp re("[A-Z0-9]{3,6}");
	_callsign_validator = new QRegExpValidator(re);
	_callsign->setValidator(_callsign_validator);
	_formLayout->addRow(_callsignLabel, _callsign);

	_layout->addLayout(_formLayout);

	_read = new QPushButton(QIcon(":/icons/actions/document-open.svg"), tr("Read Device"));
	_read->setToolTip(tr("Read configuration settings from the TNC (works on va2epr-tnc only)"));
	connect(_read, SIGNAL(clicked()), this, SLOT(doRead()));
	_buttonLayout->addWidget(_read);

	_program = new QPushButton(QIcon(":/icons/actions/document-save.svg"), tr("Program Device"));
	_program->setToolTip(tr("Write configuration settings to the TNC"));
	connect(_program, SIGNAL(clicked()), this, SLOT(doProgram()));
	_buttonLayout->addWidget(_program);

	_layout->addLayout(_buttonLayout);

	setLayout(_layout);

	_console = console;

	qDebug() << "Settings::Settings() Complete";
}

/**
 * Process clicks of the program device button.
 */
void Settings::doProgram(void) {

	qDebug() << "Settings::doProgram() Enter";

	QString ds = "{Sd:";
	unsigned char d = _tx_delay->text().toInt();
	ds += d;
	ds += "}";
	qDebug() << "Sending '" << ds << "'"; 
	_console->writePort(ds);

	QString qs = "{Sp:";
	unsigned char p = _p->text().toInt();
	qs += p;
	qs += "}";
	qDebug() << "Sending '" << qs << "'"; 
	_console->writePort(qs);

	QString ss = "{Ss:";
	unsigned char slot_time = _slot_time->text().toInt();
	ss += slot_time;
	ss += "}";
	qDebug() << "Sending '" << ss << "'"; 
	_console->writePort(ss);

	QString ts = "{St:";
	unsigned char tx_tail = _tx_tail->text().toInt();
	ts += tx_tail;
	ts += "}";
	qDebug() << "Sending '" << ts << "'"; 
	_console->writePort(ts);

	QString fs = "{Sf:";
	unsigned char full_duplex = (_full_duplex->checkState() == Qt::Checked);
	fs += full_duplex;
	fs += "}";
	qDebug() << "Sending '" << fs << "'"; 
	_console->writePort(fs);

	QString cs = "{Sc:";
	cs += _callsign->text();
	cs += "}";
	qDebug() << "Sending '" << cs << "'"; 
	_console->writePort(cs);

	_console->writePort("{W}");

	qDebug() << "Settings::doProgram() Complete";
}

/**
 * Process clicks of the read device button.
 * Go into polling mode, send get commands, wait 25ms, do read, populate fields, repeat.
 */
void Settings::doRead(void) {

	qDebug() << "Settings::doRead() Enter";

	_console->setPortMode(QextSerialPort::Polling);

	/* do read here to empty read buffer so we don't get garbage */
	_console->readPort();

	_console->writePort("{Gd}");
	usleep(25000);
	QString ds = _console->readPort();
	qDebug() << "Read " << ds;
	if (ds.indexOf(":") != -1 && ds.indexOf("}") != -1) {
		ds = ds.mid(ds.indexOf(":") + 1, ds.indexOf("}") - 1 - ds.indexOf(":"));
		_tx_delay->setText(ds);
	}

	_console->writePort("{Gp}");
	usleep(25000);
	QString ps = _console->readPort();
	qDebug() << "Read " << ps;
	if (ps.indexOf(":") != -1 && ps.indexOf("}") != -1) {
		ps = ps.mid(ps.indexOf(":") + 1, ps.indexOf("}") - 1 - ps.indexOf(":"));
		_p->setText(ps);
	}

	_console->writePort("{Gs}");
	usleep(25000);
	QString ss = _console->readPort();
	qDebug() << "Read " << ss;
	if (ss.indexOf(":") != -1 && ss.indexOf("}") != -1) {
		ss = ss.mid(ss.indexOf(":") + 1, ss.indexOf("}") - 1 - ss.indexOf(":"));
		_slot_time->setText(ss);
	}

	_console->writePort("{Gt}");
	usleep(25000);
	QString ts = _console->readPort();
	qDebug() << "Read " << ts;
	if (ts.indexOf(":") != -1 && ts.indexOf("}") != -1) {
		ts = ts.mid(ts.indexOf(":") + 1, ts.indexOf("}") - 1 - ts.indexOf(":"));
		_tx_tail->setText(ts);
	}

	_console->writePort("{Gf}");
	usleep(25000);
	QString fs = _console->readPort();
	qDebug() << "Read " << fs;
	if (fs.indexOf(":") != -1 && fs.indexOf("}") != -1) {
		fs = fs.mid(fs.indexOf(":") + 1, fs.indexOf("}") - 1 - fs.indexOf(":"));
		_full_duplex->setCheckState((fs.at(0) == '\0') ? Qt::Unchecked : Qt::Checked);
	}

	_console->writePort("{Gc}");
	usleep(25000);
	QString cs = _console->readPort();
	qDebug() << "Read " << cs;
	if (cs.indexOf(":") != -1 && cs.indexOf("}") != -1) {
		cs = cs.mid(cs.indexOf(":") + 1, cs.indexOf("}") - 1 - cs.indexOf(":"));
		_callsign->setText(cs);
	}

	_console->setPortMode(QextSerialPort::EventDriven);

	qDebug() << "Settings::doRead() Complete";

}
