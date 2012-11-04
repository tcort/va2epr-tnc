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

/* GLOBAL INCLUDES */

#include <QtGui>

/* LOCAL INCLUDES */

#include "AboutDialog.h"
#include "main.h"

/**
 * The about dialog for this program. It displays some copyright info, version info, and a link to the project website.
 */
AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent, Qt::Dialog) {

	setWindowTitle(tr("About %1").arg(PROGRAM_NAME));

	QVBoxLayout *layout = new QVBoxLayout(this);

	QLabel *title = new QLabel(tr("%1").arg(PROGRAM_NAME));

	QFont font = title->font();
	font.setPointSize(font.pointSize() + 4);
	font.setWeight(QFont::Bold);
	title->setFont(font);
	title->setMargin(4);
	title->setAlignment(Qt::AlignCenter);
	layout->addWidget(title);

	QLabel *copyright = new QLabel(tr("%1").arg(PROGRAM_COPYRIGHT));
	copyright->setAlignment(Qt::AlignCenter);
	layout->addWidget(copyright);

	QLabel *program_version = new QLabel(tr("Version %1").arg(PROGRAM_VERSION));
	program_version->setAlignment(Qt::AlignCenter);
	layout->addWidget(program_version);

	QLabel *homepage = new QLabel(tr("<a href=\"%1\">%2</a>").arg(PROGRAM_HOMEPAGE).arg(PROGRAM_HOMEPAGE));
	homepage->setOpenExternalLinks(true);
	homepage->setAlignment(Qt::AlignCenter);
	layout->addWidget(homepage);

	QPushButton *close = new QPushButton;
	close->setText(tr("Close"));
	layout->connect(close, SIGNAL(clicked()), this, SLOT(close()));
	layout->addWidget(close);

	setLayout(layout);
}
