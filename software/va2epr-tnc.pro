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

HEADERS += AboutDialog.h   ConnectDialog.h   Console.h   main.h   Settings.h   va2epr_tnc.h
SOURCES += AboutDialog.cpp ConnectDialog.cpp Console.cpp main.cpp Settings.cpp va2epr_tnc.cpp

# Libraries

LIBS += -lqextserialport-1.2	# QextSerialPort - code.google.com/p/qextserialport

# Definitions

DEFINES += QEXTSERIALPORT_USING_SHARED

# Words

TRANSLATIONS =	locale/fr.ts

# Resources

RESOURCES =	html.qrc \
		icons.qrc \
		locale.qrc

# Docs

docs.depends = $(SOURCES)
docs.commands = doxygen
QMAKE_EXTRA_TARGETS += docs

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
