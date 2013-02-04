# va2epr-tnc software - terminal node controller software for va2epr-tnc
# Copyright (C) 2012, 2013 Thomas Cort <va2epr@rac.ca>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TEMPLATE = app
TARGET = va2epr-tnc
DEPENDPATH += .
INCLUDEPATH += .

QT += webkit

# Code

HEADERS += AboutDialog.h   Console.h   main.h   ReaderThread.h   SerialPort.h   Settings.h   va2epr_tnc.h
SOURCES += AboutDialog.cpp Console.cpp main.cpp ReaderThread.cpp SerialPort.cpp Settings.cpp va2epr_tnc.cpp

# Words

TRANSLATIONS =	locale/fr.ts

# Resources

RESOURCES =	html.qrc \
		icons.qrc \
		locale.qrc

# Icons

win32 {
	RC_FILE = va2epr-tnc.rc
}

macosx {
	ICON = va2epr-tnc.icns
}

symbian {
	ICON = va2epr-tnc.svg
}
