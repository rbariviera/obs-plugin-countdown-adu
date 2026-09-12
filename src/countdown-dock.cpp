/*
obs-plugin-countdown-adu
Copyright (C) 2026

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "countdown-dock.hpp"

#include <obs-module.h>

#include <QLabel>
#include <QVBoxLayout>

CountdownDock::CountdownDock(QWidget *parent) : QWidget(parent)
{
	setObjectName("countdownAduDockWidget");

	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(12, 12, 12, 12);

	label = new QLabel(obs_module_text("CountdownAdu.Dock.Placeholder"), this);
	label->setAlignment(Qt::AlignCenter);
	label->setWordWrap(true);

	layout->addWidget(label);
	layout->addStretch(1);

	setLayout(layout);
}
