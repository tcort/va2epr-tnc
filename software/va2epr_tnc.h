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

#ifndef __VA2EPR_TNC_H
#define __VA2EPR_TNC_H

#include <QtGui>
#include <QObject>
#include <QtWebKit>

#include "Console.h"

class va2epr_tnc : public QMainWindow {

	Q_OBJECT

	public:
		va2epr_tnc(void);
		~va2epr_tnc(void);

	private slots:
		void doAbout(void);

	private:
		QWidget *_widget;
		QVBoxLayout *_layout;

		QMenu *_file;
		QAction *_quitAction;
		QMenu *_help;
		QAction *_helpAction;

		QWebView *_www;
		Console *_console;
		QTabWidget *_tabs;

		QLabel *_status;
};

#endif
