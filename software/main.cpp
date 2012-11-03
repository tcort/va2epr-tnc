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

#include "main.h"
#include "va2epr_tnc.h"

int main(int argc, char *argv[]) {

	QApplication app(argc, argv);

	app.setApplicationName(PROGRAM_NAME);
	app.setApplicationVersion(PROGRAM_VERSION);
	app.setOrganizationName(ORG_NAME);
	app.setOrganizationDomain(ORG_DOMAIN);

	QTranslator qtTranslator;
        qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qtTranslator);

	QTranslator myappTranslator;
	myappTranslator.load(QString(":/locale/%1.qm").arg(QLocale::system().name()));
	app.installTranslator(&myappTranslator);

	va2epr_tnc tnc;
	tnc.show();

	return app.exec();
}
