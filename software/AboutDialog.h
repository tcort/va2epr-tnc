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

#ifndef __ABOUT_DIALOG_H
#define __ABOUT_DIALOG_H

#include <QtGui>
#include <QObject>

/**
 * Standard About Dialog Box (Viewable under the Help->About menu in the GUI).
 */
class AboutDialog : public QDialog {
	Q_OBJECT

	public:
		/**
		 * The about dialog for this program. It displays some copyright info, version info, and a link to the project website.
		 */
		AboutDialog(QWidget * parent = 0);
};

#endif
